#ifndef PARSE_ERROR_HPP_
#define PARSE_ERROR_HPP_

#include "source_location.hpp"

#include <exception>
#include <string>
#include <sstream>

struct parse_error
  : std::exception
{
    parse_error(const std::string& msg, const source_position& position, const char* file_name)
    {
        std::ostringstream oss;
        if(file_name)
            oss << file_name << ":";
        oss << position.line << ":";
        oss << position.line_position << ": ";
        oss << msg;

        error = oss.str();
    }
    
    std::string error;

    virtual const char* what() const noexcept
    {
        return error.c_str();
    }
};

#endif

