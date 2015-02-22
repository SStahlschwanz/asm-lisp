#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse_state.hpp"
#include "parse.hpp"

#include <string>
#include <unordered_map>
#include <vector>


typedef std::unordered_map<std::string, const symbol*> export_table;
struct module
{
    export_table exports;
    list_symbol syntax_tree;
    std::vector<std::unique_ptr<symbol>> evaluated_exports; // owns from macros evaluated exports
};

std::unordered_map<std::string, std::vector<symbol_source>> 
        get_imported_modules(const list_symbol& syntax_tree);
module evaluate_module(list_symbol syntax_tree,
        const std::unordered_map<std::string, module>& dependencies);

#endif

