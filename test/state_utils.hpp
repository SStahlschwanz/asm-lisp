#ifndef STATE_UTILS_HPP_
#define STATE_UTILS_HPP_

#include <string>

#include "graph_building.hpp"
#include "../src/parse_state.hpp"

typedef parse_state<const char*> state;

inline std::string remaining(state s)
{
    std::string result;
    while(!s.empty())
    {
        result += s.front();
        s.pop_front();
    }
    return result;
}


constexpr size_t default_file_id = 123;

inline state make_state(const char* str)
{
    const char* begin = str;
    const char* end = str;
    while(*end != 0)
        ++end;
    return state{begin, end, default_file_id, create_graph()};
}

#endif

