#ifndef COMPILE_MACRO_HPP_
#define COMPILE_MACRO_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"

macro_symbol compile_macro(list_symbol::const_iterator begin, list_symbol::const_iterator end,
        compilation_context& context);

#endif

