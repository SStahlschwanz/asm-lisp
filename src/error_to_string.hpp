#ifndef ERROR_TO_STRING_HPP_
#define ERROR_TO_STRING_HPP_

#include <string>
#include <functional>
#include <ostream>

#include "error/compile_exception.hpp"
#include "compilation_context.hpp"
#include "symbol.hpp"

std::string default_error_to_string(const compile_exception& exc, std::function<std::string (std::size_t)> file_id_to_name);

std::ostream& print_symbol(std::ostream& os, const symbol& s, compilation_context& context);

#endif

