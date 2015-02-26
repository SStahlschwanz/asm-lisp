#ifndef STATE_UTILS_HPP_
#define STATE_UTILS_HPP_

#include <string>

#include "../src/parse_state.hpp"
#include "../src/compilation_context.hpp"

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

compilation_context context; // this file may only be included once because of this
identifier_id_t operator""_id(const char* str, size_t /*length*/)
{
    return context.identifier_id(str);
}

constexpr size_t default_file_id = 123;
state make_state(const char* str)
{
    const char* begin = str;
    const char* end = str;
    while(*end != 0)
        ++end;
    return state{begin, end, default_file_id, context};
}

#endif

