#ifndef NODE_SOURCE_HPP_
#define NODE_SOURCE_HPP_

#include <boost/variant.hpp>

#include <cstddef>

struct file_position
{
    std::size_t line;
    std::size_t line_pos;
};

struct file_source
{
    file_position begin;
    file_position end;
    size_t file_id;
};

typedef boost::variant<boost::blank, file_source> node_source;

#endif

