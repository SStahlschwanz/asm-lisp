#ifndef PARSE_REFERENCE_HPP_
#define PARSE_REFERENCE_HPP_

#include "symbol.hpp"

#include <boost/optional.hpp>

namespace parse_reference_detail
{

inline bool is_letter(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
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
boost::optional<ref_symbol> parse_reference(State& state)
{
    using namespace parse_reference_detail;
    if(state.empty())
        return boost::none;
    else if(is_letter(state.front()))
    {
        std::string identifier;
        file_position begin = state.position();

        while(!state.empty() && is_alpha_numeric(state.front()))
        {
            identifier.push_back(state.front());
            state.pop_front();
        }
        
        file_position end = state.position();
        identifier_id_t identifier_id = state.identifier_id(identifier);
        #ifdef NDEBUG
        ref_symbol result{identifier_id};
        #else
        ref_symbol result{identifier_id, identifier, 0}; 
        #endif
        result.source(file_source{begin, end, state.file()});
        return result;
    }
    else if(is_operator(state.front()))
    {
        std::string identifier;
        file_position begin = state.position();

        while(!state.empty() && is_operator(state.front()))
        {
            identifier.push_back(state.front());
            state.pop_front();
        }

        file_position end = state.position();
        identifier_id_t identifier_id = state.identifier_id(identifier);

        ref_symbol result{identifier_id};
        result.source(file_source{begin, end, state.file()});
        return result;
    }
    else
        return boost::none;
}

#endif

