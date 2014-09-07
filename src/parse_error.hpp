#ifndef PARSE_ERROR_HPP_
#define PARSE_ERROR_HPP_

#include "symbol.hpp"

#include <exception>
#include <string>
#include <sstream>

inline std::ostream& operator<<(std::ostream& os, const source_location& location)
{
    if(location.file_name)
        os << location.file_name << ":";
    os << location.line;
    os << ":";
    os << location.pos;
    return os;
}

struct parse_error
  : std::exception
{
    parse_error(const source_location& location, const std::string& msg)
    {
        std::ostringstream oss;
        oss << location << ":" << msg;
        error = oss.str();
    }
    
    std::string error;

    virtual const char* what() const noexcept
    {
        return error.c_str();
    }
};

#endif

