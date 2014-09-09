#ifndef SOURCE_LOCATION_HPP_
#define SOURCE_LOCATION_HPP_

#include <stddef.h>

struct source_position
{
    size_t line;
    size_t line_position;
};

struct source_range
{
    source_position begin;
    source_position end;
    const char* file_name;
};

#endif

