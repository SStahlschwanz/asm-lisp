#ifndef PARSE_NODE_HPP_
#define PARSE_NODE_HPP_

#include "symbol.hpp"
#include "parse_error.hpp"
#include "parse_literal.hpp"
#include "parse_reference.hpp"
#include "whitespace.hpp"

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
                throw parse_error(state.location(), "expected \";\"");
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
        whitespace(state);
        while( (semicolon_list = parse_semicolon_list(state)) )
        {
            result.push_back(std::move(*semicolon_list));
            whitespace(state);
        }
        
        if(state.empty() || state.front() != '}')
            throw parse_error(begin, "umatched \"{\"");
        else
        {
            state.pop_front();
            return symbol{begin, state.location(), std::move(result)};
        }
    }
}

template <class State>
boost::optional<symbol> parse_square_list(State& state)
{
    if(state.empty() || state.front() != '[')
        return boost::none;
    else
    {
        source_location begin = state.location();

        symbol::list result;
        do
        {
            state.pop_front(); // the first time, this will pop '[', afterwards ','
            whitespace(state);
            source_location before_nodes = state.location();
            
            symbol::list current_list = parse_nodes(state);
            result.push_back(symbol{before_nodes, state.location(), std::move(current_list)});
        } while(!state.empty() && state.front() == ',');
        
        if(state.empty() || state.front() != ']')
            throw parse_error(begin, "unmatched \"[\"");
        else
        {
            state.pop_front();
            return symbol{begin, state.location(), std::move(result)};
        }
    }
}

template <class State>
boost::optional<symbol> parse_round_list(State& state)
{
    if(state.empty() || state.front() != '(')
        return boost::none;
    else
    {
        source_location begin = state.location();
        state.pop_front();

        whitespace(state);
        source_location first_begin = state.location();
        symbol::list first_list = parse_nodes(state);

        if(state.empty())
            throw parse_error(begin, "unmatched \"(\"");
        else if(state.front() == ')')
        {
            state.pop_front();
            return symbol{begin, state.location(), std::move(first_list)};
        }
        else if(state.front() == ',')
        {
            symbol::list result;
            result.push_back(symbol{first_begin, state.location(), std::move(first_list)});
            do
            {
                state.pop_front();
                whitespace(state);
                source_location before = state.location();
                symbol::list current_list = parse_nodes(state);
                result.push_back(symbol{before, state.location(), std::move(current_list)});
            } while(!state.empty() && state.front() == ',');
            
            if(state.empty() || state.front() != ')')
                throw parse_error(begin, "unmatched \"(\"");
            else
            {
                state.pop_front();
                return symbol{begin, state.location(), std::move(result)};
            }
        }
        else
            throw parse_error(begin, "unmatched \"(\"");
    }
}

template <class State>
boost::optional<symbol> parse_node(State& state)
{
    boost::optional<symbol> result;
    if( (result = parse_literal(state)) )
        return result;
    else if( (result = parse_reference(state)) )
        return result;
    else if( (result = parse_curly_list(state)) )
        return result;
    else if( (result = parse_square_list(state)) )
        return result;
    else if( (result = parse_round_list(state)) )
        return result;
    else
        return boost::none;
}

#endif

