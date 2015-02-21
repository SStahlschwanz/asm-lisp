#ifndef PARSE_LITERAL_HPP_
#define PARSE_LITERAL_HPP_

#include "symbol.hpp"
#include "error/parse_error.hpp"

#include <boost/optional.hpp>

namespace parse_literal_detail
{

inline bool is_digit(char c)
{
    return '0' <= c && c <= '9';
}

}

template <class State>
boost::optional<lit_symbol> parse_literal(State& state)
{
    using namespace parse_literal_detail;
    
    if(state.empty())
        return boost::none;
    else if(state.front() == '"')
    {
        file_position begin = state.position();
        lit_symbol result;
        
        state.pop_front();
        while(true)
        {
            if(state.empty() || state.front() == '\n')
                throw parse_exception(begin, parse_error::UNMATCHED_QUOTE);
            if(state.front() == '"')
                break;
            else
            {
                result.push_back(state.front());
                state.pop_front();
            }
        }
        
        state.pop_front();
        
        file_position end = state.position();
        result.source(file_source{begin, end, state.file()});
        return result;
    }
    else if(is_digit(state.front()))
    {
        file_position begin = state.position();
        lit_symbol result;
        result.push_back(state.front());
        
        state.pop_front();
        while(!state.empty() && is_digit(state.front()))
        {
            result.push_back(state.front());
            state.pop_front();
        }
        file_position end = state.position();
        result.source(file_source{begin, end, state.file()});
        return result;
    }
    else
        return boost::none;
}

#endif

