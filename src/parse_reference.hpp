#ifndef PARSE_REFERENCE_HPP_
#define PARSE_REFERENCE_HPP_

#include "symbol.hpp"

#include <boost/optional.hpp>

namespace parse_reference_detail
{

bool is_letter(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
bool is_alpha_numeric(char c)
{
    return is_letter(c) || ('0' <= c && c <= '9');
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
        source_location begin = state.location();

        while(!state.empty() && is_alpha_numeric(state.front()))
        {
            result.identifier += state.front();
            state.pop_front();
        }
        
        source_location end = state.location();
        return symbol{begin, end, result};
    }
    else
        return boost::none;
}

#endif

