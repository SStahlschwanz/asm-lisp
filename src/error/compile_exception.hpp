#ifndef COMPILE_EXCEPTION_HPP_
#define COMPILE_EXCEPTION_HPP_

#include <exception>
#include <utility>
#include <vector>
#include <stdexcept>

#include <boost/variant.hpp>

#include "../symbol_source.hpp"

struct code_location
{
    file_position pos;
    std::size_t file_id;
};

typedef boost::variant<code_location, std::string, size_t> error_parameter;

struct to_error_parameter_visitor
  : boost::static_visitor<error_parameter>
{
    error_parameter operator()(const boost::blank&)
    {
        return std::string{"<no source>"};
    }
    error_parameter operator()(const file_source& src)
    {
        return code_location{src.begin, src.file_id};
    }
};

inline error_parameter to_error_parameter(const symbol_source& src)
{
    to_error_parameter_visitor visitor;
    return boost::apply_visitor(visitor, src);
}

class compile_exception
  : public std::exception
{
public:
    template<class... Types>
    compile_exception(const char* error_name, Types&&... printable_objects)
      : error_name(error_name),
        parameters{std::forward<Types>(printable_objects)...}
    {}
    
    const char* error_name;
    std::vector<error_parameter> parameters;
};

struct not_implemented
  : std::runtime_error
{
   using std::runtime_error::runtime_error;
};


#endif

