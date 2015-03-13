#ifndef MACRO_EXECUTION_HPP_
#define MACRO_EXECUTION_HPP_

#include "symbol.hpp"

#include <cstddef>
#include <utility>
#include <vector>

std::pair<any_symbol, std::vector<std::unique_ptr<any_symbol>>> execute_macro(std::size_t (*function_ptr)(std::size_t), list_symbol::const_iterator args_begin, list_symbol::const_iterator args_end);


#endif

