#ifndef SOURCE_LOCATION_HPP_
#define SOURCE_LOCATION_HPP_

#include <stddef.h>
#include <ostream>

struct source_position
{
    size_t line;
    size_t line_position;
};

inline std::ostream& operator<<(std::ostream& os, const source_position& pos)
{
    os << pos.line << ":" << pos.line_position;
    return os;
}

struct source_range
{
    source_position begin;
    source_position end;
    const char* file_name;
};

inline std::ostream& operator<<(std::ostream& os, const source_range& range)
{
    if(range.file_name)
        os << range.file_name;
    os << range.begin;
    // do not output end by default
    return os;
}


#endif

