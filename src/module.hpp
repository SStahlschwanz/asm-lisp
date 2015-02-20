#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse_state.hpp"
#include "parse.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <iterator>

struct module
{
    typedef std::unordered_map<std::string, const symbol*> export_table;
    export_table exports;
    
    symbol::list syntax_tree;
    std::vector<std::unique_ptr<symbol>> evaluated_exports; // holding from macros evaluated exported symbols
    
    template<class IteratorType>
    static module read(parse_state<IteratorType>& state)
    {
        module result;
        result.syntax_tree = parse_file(state);
        return result;
    }
    
    std::unordered_map<std::string, symbol::source_type> get_imported_modules() const;
    void evaluate_exports(const std::unordered_map<std::string, const module*>& module_map);
};

#endif

