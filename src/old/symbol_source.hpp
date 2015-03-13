#ifndef SYMBOL_SOURCE_HPP_
#define SYMBOL_SOURCE_HPP_

#include <boost/variant.hpp>

struct file_position
{
    size_t line;
    size_t line_pos;
};

struct file_source
{
    file_position begin;
    file_position end;
    size_t file_id;
};

typedef boost::variant<boost::blank, file_source> symbol_source;

#endif

