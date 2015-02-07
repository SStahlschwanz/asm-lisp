#ifndef COMPILATION_UNIT_HPP_
#define COMPILATION_UNIT_HPP_

#include "module.hpp"

#include <string>
#include <unordered_map>
#include <vector>

struct compilation_unit
{
    std::unordered_map<std::string, module> modules;
};


compilation_unit compile(const std::vector<std::string>& files);

#endif

