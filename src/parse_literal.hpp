#ifndef PARSE_LITERAL_HPP_
#define PARSE_LITERAL_HPP_

#include "symbol.hpp"
#include "parse_error.hpp"

#include <boost/optional.hpp>

namespace parse_literal_detail
{

inline bool is_digit(char c)
{
    return '0' <= c && c <= '9';
}

}

template <class State>
boost::optional<symbol> parse_literal(State& state)
{
    using namespace parse_literal_detail;
    
    if(state.empty())
        return boost::none;
    else if(state.front() == '"')
    {
        source_position begin = state.position();
        symbol::literal result; // std::string
        
        state.pop_front();
        while(true)
        {
            if(state.empty() || state.front() == '\n')
                throw parse_error("unmatched \"", begin, state.file());
            if(state.front() == '"')
                break;
            else
            {
                result += state.front();
                state.pop_front();
            }
        }
        
        state.pop_front();
        
        source_position end = state.position();
        return symbol{std::move(result), source_range{begin, end, state.file()}};
    }
    else if(is_digit(state.front()))
    {
        source_position begin = state.position();
        symbol::literal result; // std::string
        result += state.front();
        
        state.pop_front();
        while(!state.empty() && is_digit(state.front()))
        {
            result += state.front();
            state.pop_front();
        }
        source_position end = state.position();
        return symbol{std::move(result), source_range{begin, end, state.file()}};
    }
    else
        return boost::none;
}

#endif

