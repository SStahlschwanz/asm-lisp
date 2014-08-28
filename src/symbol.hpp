#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include <boost/variant.hpp>

struct source_location
{
    size_t line;
    size_t pos;
};

class symbol
{
public:
    typedef std::string literal;
    struct reference
    {
        std::string identifier;
    };
    typedef std::vector<symbol> list;
    
    source_location location_begin;
    source_location location_end;

    boost::variant<literal, reference, list> content;
};

#endif

