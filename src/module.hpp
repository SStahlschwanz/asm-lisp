#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <istream>

struct module
{
    std::vector<std::string> required_modules;
    symbol::list syntax_tree;
    std::unordered_map<std::string, symbol> defined_symbols;
};

module read_module(std::istream& file_name);
void dispatch_and_eval(module& m, const std::vector<const module*>& dependencies);

#endif

