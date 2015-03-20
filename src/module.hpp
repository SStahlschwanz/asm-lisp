#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "node.hpp"
//#include "parse_state.hpp"
//#include "parse.hpp"
#include "compilation_context.hpp"

#include <boost/optional.hpp>

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <functional>


struct import_statement
{
    const list_node& statement;
    const lit_node& imported_module;
    const list_node& import_list;
};
struct export_statement
{
    const list_node& statement;
};
struct module_header
{
    std::vector<import_statement> imports;
    std::vector<export_statement> exports;
};
module_header read_module_header(const list_node& syntax_tree);
std::unordered_map<std::string, std::vector<node_source>> imported_modules(const module_header& header);

typedef std::unordered_map<std::string, const node&> symbol_table;
struct module
{
    dynamic_graph node_owner;
    symbol_table exports;
};
module evaluate_module(list_node& syntax_tree, dynamic_graph graph_owner, const module_header& header, std::function<const module& (const import_statement&)> get_module_func);

#endif

