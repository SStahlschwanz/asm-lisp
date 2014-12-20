#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse.hpp"

#include <string>
#include <vector>

struct module
{
    std::string file_name;
    std::vector<std::string> required_modules;
    symbol::list contents;
    std::list<symbol> additional_symbols;
};

symbol::list parse_module(const std::string& file_name);
std::vector<std::string> required_modules(const symbol::list& parsed_module);
void dispatch_references(module& mod);

#endif

