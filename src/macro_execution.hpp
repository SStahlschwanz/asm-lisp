#ifndef MACRO_EXECUTION_HPP_
#define MACRO_EXECUTION_HPP_

#include "node.hpp"

#include <cstddef>
#include <utility>
#include <vector>

typedef std::size_t macro_func(std::size_t);

std::pair<const node&, dynamic_graph> execute_macro(macro_func* func, node_range args);


#endif

