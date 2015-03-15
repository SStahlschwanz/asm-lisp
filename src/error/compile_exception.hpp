#ifndef COMPILE_EXCEPTION_HPP_
#define COMPILE_EXCEPTION_HPP_

#include <exception>
#include <utility>
#include <vector>
#include <stdexcept>

#include <boost/variant.hpp>

#include "../node_source.hpp"
#include "error_kind.hpp"
#include <mblib/boost_variant.hpp>

namespace llvm
{
class Type;
}

struct code_location
{
    file_position pos;
    std::size_t file_id;
};
typedef boost::variant<boost::blank, code_location> error_location;


inline error_location to_error_location(const node_source& src)
{
    return visit<error_location>(src,
    [&](boost::blank)
    {
        return boost::blank();
    },
    [&](const file_source& src)
    {
        return code_location{src.begin, src.file_id};
    });
}
template<class T>
inline error_location to_error_location(T&& obj)
{
    return std::forward<T>(obj);
}

typedef boost::variant<boost::blank, std::size_t, std::string, llvm::Type&> error_parameter;

struct compile_exception
  : std::exception
{
    template<class Location, class... ParamTypes>
    compile_exception(error_kind kind, std::size_t error_id, const Location& location, ParamTypes&&... params)
      : kind{kind},
        error_id{error_id},
        location{to_error_location(location)},
        params{std::forward<ParamTypes>(params)...}
    {}
    
#ifndef NDEBUG
    const char* what() const noexcept override;
    // implemented in printing.cpp
    // 'leaks' memory (freed when program terminates)
#endif

    error_kind kind;
    std::size_t error_id;
    error_location location;
    std::vector<error_parameter> params;
};

struct not_implemented
  : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

#endif

