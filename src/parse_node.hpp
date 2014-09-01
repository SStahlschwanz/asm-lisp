#ifndef PARSE_NODE_HPP_
#define PARSE_NODE_HPP_

#include "symbol.hpp"
#include "parse_error.hpp"

#include <boost/optional.hpp>

template <class State>
boost::optional<symbol> parse_node(State& state);

template <class State>
typename symbol::list parse_nodes(State& state)
{
    typename symbol::list result;

    boost::optional<symbol> parse_result;
    while( (parse_result = parse_node(state)) )
    {
        result.push_back(std::move(*parse_result));
        whitespace(state);
    }
    return result;
}

template <class State>
boost::optional<symbol> parse_semicolon_list(State& state)
{
    if(state.empty())
        return boost::none;
    else if(state.front() == ';')
    {
        source_location begin = state.location();
        state.pop_front();
        return symbol{begin, state.location(), typename symbol::list()};
    }
    else
    {
        source_location begin = state.location();

        typename symbol::list result = parse_nodes(state);
        if(state.empty() || state.front() != ';')
        {
            if(result.empty())
                return boost::none;
            else
                throw parse_error();
        }
        else
        {
            state.pop_front();
            return symbol{begin, state.location(), std::move(result)};
        }
    }
}

template <class State>
boost::optional<symbol> parse_curly_list(State& state)
{
    if(state.empty() || state.front() != '{')
        return boost::none;
    else
    {
        source_location begin = state.location();
        state.pop_front();
        typename symbol::list result;
        boost::optional<symbol> semicolon_list;
        while( (semicolon_list = parse_semicolon_list(state)) )
            result.push_back(std::move(*semicolon_list));
        
        if(state.empty() || state.front() != '}')
            throw parse_error();
        else
        {
            state.pop_front();
            return symbol{begin, state.location(), std::move(result)};
        }
    }
}

#endif

