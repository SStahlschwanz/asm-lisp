#ifndef STATE_UTILS_HPP_
#define STATE_UTILS_HPP_

#include <string>

#include "../src/parse_state.hpp"

typedef parse_state<const char*> state;

std::string remaining(state s)
{
    std::string result;
    while(!s.empty())
    {
        result += s.front();
        s.pop_front();
    }
    return result;
}

state make_state(const char* str)
{
    const char* begin = str;
    const char* end = str;
    while(*end != 0)
        ++end;
    return state{begin, end};
}



#endif

