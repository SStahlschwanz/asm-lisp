#ifndef PARSE_REFERENCE_HPP_
#define PARSE_REFERENCE_HPP_

#include "symbol.hpp"

#include <boost/optional.hpp>

namespace parse_reference_detail
{

inline bool is_letter(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
inline bool is_alpha_numeric(char c)
{
    return is_letter(c) || ('0' <= c && c <= '9');
}
inline bool is_operator(char c)
{
    static const char operators[] = "+-*^/%~<>=!|&.$";
    static const char* end = operators + sizeof(operators) - 1; // zero-terminated string
    return std::find(operators, end, c) != end;
}

}

template <class State>
boost::optional<symbol> parse_reference(State& state)
{
    using namespace parse_reference_detail;
    if(state.empty())
        return boost::none;
    else if(is_letter(state.front()))
    {
        symbol::reference result;
        source_position begin = state.position();

        while(!state.empty() && is_alpha_numeric(state.front()))
        {
            result.identifier += state.front();
            state.pop_front();
        }
        
        source_position end = state.position();
        return symbol{std::move(result), source_range{begin, end, state.file()}};
    }
    else if(is_operator(state.front()))
    {
        symbol::reference result;
        source_position begin = state.position();

        while(!state.empty() && is_operator(state.front()))
        {
            result.identifier += state.front();
            state.pop_front();
        }

        source_position end = state.position();
        return symbol{std::move(result), source_range{begin, end, state.file()}};
    }
    else
        return boost::none;
}

#endif

