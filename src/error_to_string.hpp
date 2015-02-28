#ifndef ERROR_TO_STRING_HPP_
#define ERROR_TO_STRING_HPP_

#include <string>
#include <unordered_map>

#include "error/compile_exception.hpp"

std::string default_error_to_string(const compile_exception& exc, const std::unordered_map<std::size_t, std::string>& file_id_name_mapping);

#endif

