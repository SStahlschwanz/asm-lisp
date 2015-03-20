#include "dynamic_graph.hpp"

#include "node.hpp"

#include <mblib/boost_variant.hpp>

#include <unordered_map>
#include <limits>

using boost::get;

using std::make_unique;
using std::move;
using std::string;
using std::pair;
using std::make_pair;
using std::vector;
using std::back_inserter;
using std::unordered_map;
using std::numeric_limits;
using std::size_t;

id_node& dynamic_graph::create_id(size_t id)
{
    auto storage = make_unique<node_data>(id_node{id});
    id_node& result = get<id_node>(*storage);
    data.push_back(std::move(storage));

    return result;
}

lit_node& dynamic_graph::create_lit(string str)
{
    auto storage = make_unique<node_data>(make_pair(lit_node{nullptr, nullptr}, move(str)));
    lit_data& result_data = get<lit_data>(*storage);
    data.push_back(std::move(storage));

    lit_node& lit = result_data.first;
    std::string& s = result_data.second;
    char* begin = &s[0];
    char* end = begin + s.size();
    lit = lit_node{begin, end};
    return lit;
}

ref_node& dynamic_graph::create_ref(string str)
{
    auto storage = make_unique<node_data>(make_pair(ref_node{nullptr, nullptr, nullptr}, move(str)));
    ref_data& result_data = get<ref_data>(*storage);
    data.push_back(std::move(storage));

    ref_node& ref = result_data.first;
    std::string& s = result_data.second;
    char* begin = &s[0];
    char* end = begin + s.size();
    ref = ref_node{begin, end, nullptr};
    return ref;
}

list_node& dynamic_graph::create_list(vector<node*> nodes)
{
    auto storage = make_unique<node_data>(make_pair(list_node{nullptr, nullptr}, move(nodes)));
    list_data& result_data = get<list_data>(*storage);
    data.push_back(std::move(storage));

    list_node& list = result_data.first;
    vector<node*>& v = result_data.second;
    node** begin = &v[0];
    node** end = begin + v.size();
    list = list_node{begin, end};
    return list;
}

macro_node& dynamic_graph::create_macro()
{
    auto storage = make_unique<node_data>(macro_node{{}});
    macro_node& result = get<macro_node>(*storage);
    data.push_back(std::move(storage));

    return result;
}
proc_node& dynamic_graph::create_proc()
{
    auto storage = make_unique<node_data>(proc_node{nullptr, nullptr});
    proc_node& result = get<proc_node>(*storage);
    data.push_back(std::move(storage));

    return result;
}

void dynamic_graph::add(dynamic_graph graph)
{
    std::move(graph.data.begin(), graph.data.end(), back_inserter(data));
}

dynamic_graph::dynamic_graph(const node& n)
{
    unordered_map<const node*, node_data*> copied_nodes;
    vector<pair<const node*, node_data*>> node_stack;

    auto node_ptr_of_data = [&](node_data& d)
    {
        return visit<node*>(d, [&](auto& obj) -> node*
        {
            return reinterpret_cast<node*>(&obj);
        });
    };

    auto ptr_for = [&](const node& child_node) -> node*
    {
        auto it = copied_nodes.find(&child_node);
        if(it != copied_nodes.end())
            return node_ptr_of_data(*it->second);
        
        data.push_back(make_unique<node_data>(id_node{0}));
        node_data* copied_data_ptr = data.back().get();
        copied_nodes.insert({&child_node, copied_data_ptr});
        node_stack.push_back(make_pair(&child_node, copied_data_ptr));

        return node_ptr_of_data(*copied_data_ptr);
    };

    ptr_for(n);
    while(!node_stack.empty())
    {
        const node& current_node = *node_stack.back().first;
        node_data* current_data = node_stack.back().second;
        node_stack.pop_back();

        current_node.visit(
        [&](const id_node& id)
        {
            *current_data = id_node{id};
        },
        [&](const lit_node& lit)
        {
            *current_data = make_pair(lit_node{nullptr, nullptr}, "");
            lit_data& data = get<lit_data>(*current_data);
            data.second = save<string>(lit);
            char* str_begin = &data.second[0];
            char* str_end = str_begin + data.second.size();
            lit_node cloned_lit{str_begin, str_end};
            data.first = lit_node{str_begin, str_end};
        },
        [&](const ref_node& ref)
        {
            *current_data = make_pair(ref_node{nullptr, nullptr, nullptr}, "");
            ref_data& data = get<ref_data>(*current_data);
            data.second = save<string>(ref.identifier());
            char* str_begin = &data.second[0];
            char* str_end = str_begin + data.second.size();
            node* child = nullptr;
            if(ref.refered())
                child = ptr_for(*ref.refered());
            data.first = ref_node{str_begin, str_end, child};
        },
        [&](const list_node& list)
        {
            *current_data = make_pair(list_node{nullptr, nullptr}, vector<node*>{});
            list_data& data = get<list_data>(*current_data);
            data.second = save<vector<node*>>(mapped(list,
            [&](const node& n) -> node*
            {
                return ptr_for(n);
                //return nullptr;
            }));
            node** children_begin = &data.second[0];
            node** children_end = children_begin + data.second.size();
            data.first = list_node{children_begin, children_end};
        },
        [&](const macro_node& macro)
        {
            *current_data = macro_node{macro};
        },
        [&](const proc_node& proc)
        {
            *current_data = proc_node{proc};
        });
    }
}

