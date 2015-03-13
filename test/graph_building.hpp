#ifndef GRAPH_BUILDING_HPP_
#define GRAPH_BUILDING_HPP_

#include "../src/dynamic_graph.hpp"

#include <iterator>

inline dynamic_graph& create_graph()
{
    static std::vector<std::unique_ptr<dynamic_graph>> graphs;

    graphs.push_back(std::make_unique<dynamic_graph>());
    return *graphs.back();
}


struct graph_build_base
{
    node* n;
    
    graph_build_base() = default;
    graph_build_base(node& actual_node)
      : n(&actual_node)
    {}
    operator node&() const
    {
        return *n;
    }
    static dynamic_graph& graph()
    {
        static dynamic_graph& g = create_graph();
        return g;
    }
};

struct id
  : graph_build_base
{
    id(std::size_t i)
    {
        n = &graph().create_id(i);
    }

    operator id_node&() const
    {
        return n->cast<id_node>();
    }
    id_node* operator&() const
    {
        return &n->cast<id_node>();
    }
};
struct lit
  : graph_build_base
{
    lit(std::string str)
    {
        n = &graph().create_lit(std::move(str));
    }

    operator lit_node&() const
    {
        return n->cast<lit_node>();
    }
    lit_node* operator&() const
    {
        return &n->cast<lit_node>();
    }
};
struct ref
  : graph_build_base
{
    ref(std::string str = "", node* refered = nullptr)
    {
        ref_node& r = graph().create_ref(std::move(str));
        r.refered(refered);
        n = &r;
    }

    operator ref_node&() const
    {
        return n->cast<ref_node>();
    }
    ref_node* operator&() const
    {
        return &n->cast<ref_node>();
    }
};
struct list
  : graph_build_base
{
    template<class... Types>
    list(Types&&... nodes)
    {
        node* arr[] = {(&nodes)...};
        n = &graph().create_list({arr, arr + sizeof...(Types)});
    }

    operator list_node&() const
    {
        return n->cast<list_node>();
    }
    list_node* operator&() const
    {
        return &n->cast<list_node>();
    }
};

#endif

