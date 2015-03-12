#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse_state.hpp"
#include "parse.hpp"
#include "compilation_context.hpp"

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <functional>


struct import_statement
{
    const list_symbol& statement;
    const lit_symbol& imported_module;
    const list_symbol& import_list;
};
struct export_statement
{
    const list_symbol& statement;
};
struct module_header
{
    std::vector<import_statement> imports;
    std::vector<export_statement> exports;
};
module_header read_module_header(const list_symbol& syntax_tree);
std::unordered_map<std::string, std::vector<symbol_source>> imported_modules(const module_header& header);

typedef std::unordered_map<identifier_id_t, const symbol*> symbol_table;
struct module
{
    list_symbol transformed_tree;
    symbol_table exports;
    std::vector<std::unique_ptr<any_symbol>> evaluated_exports; // owns from macros evaluated exports
};
module evaluate_module(list_symbol syntax_tree, const module_header& header, std::function<const module& (const import_statement& import)> get_module_func, compilation_context& context);

#endif

