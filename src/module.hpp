#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse.hpp"

#include <string>
#include <vector>

struct module
{
    std::vector<std::string> required_modules;
    symbol::list syntax_tree;
};

module read_module(const std::string& file_name);

#endif

