#include "dynamic_graph.hpp"

#include "node.hpp"

using boost::get;

using std::make_unique;
using std::move;
using std::string;
using std::make_pair;
using std::vector;
using std::back_inserter;
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

void dynamic_graph::add(dynamic_graph graph)
{
    std::move(graph.data.begin(), graph.data.end(), back_inserter(graph.data));
}
