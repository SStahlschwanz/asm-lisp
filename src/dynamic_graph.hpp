#ifndef DYNAMIC_GRAPH_HPP_
#define DYNAMIC_GRAPH_HPP_

//#include "node.hpp"

#include <boost/variant.hpp>

#include <vector>
#include <memory>

class node;
class id_node;
class lit_node;
class ref_node;
class list_node;
class macro_node;
class proc_node;


class dynamic_graph
{
public:
    dynamic_graph() = default;

    id_node& create_id(std::size_t id);
    lit_node& create_lit(std::string str);
    ref_node& create_ref(std::string str);
    list_node& create_list(std::vector<node*> nodes);
    macro_node& create_macro();
    proc_node& create_proc();

    void add(dynamic_graph);
    node& add(const node&);

    static std::pair<node&, dynamic_graph> clone(const node&);

    typedef id_node id_data;
    typedef std::pair<lit_node, std::string> lit_data;
    typedef std::pair<ref_node, std::string> ref_data;
    typedef std::pair<list_node, std::vector<node*>> list_data;
    typedef macro_node macro_data;
    typedef proc_node proc_data;

    typedef boost::variant<id_node, lit_data, ref_data, list_data, macro_node, proc_node> node_data;

    std::vector<std::unique_ptr<node_data>> data;
};


#endif

