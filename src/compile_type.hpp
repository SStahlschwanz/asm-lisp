#ifndef COMPILE_TYPE_HPP_
#define COMPILE_TYPE_HPP_

#include "symbol.hpp"
#include "compilation_context.hpp"

type_symbol compile_int(list_symbol::const_iterator begin, list_symbol::const_iterator end,
        compilation_context& context);
type_symbol compile_struct(list_symbol::const_iterator begin, list_symbol::const_iterator end,
        compilation_context& context);

#endif

