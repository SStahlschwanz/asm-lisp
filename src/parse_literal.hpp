#ifndef PARSE_LITERAL_HPP_
#define PARSE_LITERAL_HPP_

#include "node.hpp"
#include "error/parse_error.hpp"

namespace parse_literal_detail
{

inline bool is_digit(char c)
{
    return '0' <= c && c <= '9';
}

}

template <class State>
lit_node* parse_literal(State& state)
{
    using namespace parse_literal_detail;
    using namespace parse_error;
    
    if(state.empty())
        return nullptr;
    else if(state.front() == '"')
    {
        file_position begin = state.position();
        std::string str;
        
        state.pop_front();
        while(true)
        {
            if(state.empty() || state.front() == '\n')
                fatal<id("unmatched_quote")>(code_location{begin, state.file()});
            if(state.front() == '"')
                break;
            else
            {
                str.push_back(state.front());
                state.pop_front();
            }
        }
        
        state.pop_front();
        
        file_position end = state.position();
        lit_node& lit = state.graph().create_lit(std::move(str));
        lit.source(file_source{begin, end, state.file()});
        return &lit;
    }
    else if(is_digit(state.front()))
    {
        file_position begin = state.position();
        std::string str;
        str.push_back(state.front());
        
        state.pop_front();
        while(!state.empty() && is_digit(state.front()))
        {
            str.push_back(state.front());
            state.pop_front();
        }
        file_position end = state.position();

        lit_node& lit = state.graph().create_lit(std::move(str));
        lit.source(file_source{begin, end, state.file()});
        return &lit;
    }
    else
        return nullptr;
}

#endif

