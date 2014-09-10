#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse.hpp"
#include "parse_state.hpp"

#include <fstream>
#include <stdexcept>


struct module
{
    std::string file_name;
    std::vector<std::string> required_modules;
    symbol contents;
};

module parse_module(const std::string& file_name);

#endif

