#ifndef PARSE_HPP_
#define PARSE_HPP_

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
        source_position begin = state.position();
        state.pop_front();
        return symbol{symbol::list(), source_range{begin, state.position(), state.file()}};
    }
    else
    {
        source_position begin = state.position();

        typename symbol::list result = parse_nodes(state);
        if(state.empty() || state.front() != ';')
        {
            if(result.empty())
                return boost::none;
            else
                throw parse_error("expected \";\"", state.position(), state.file());
        }
        else
        {
            state.pop_front();
            return symbol{std::move(result), source_range{begin, state.position(), state.file()}};
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
        source_position begin = state.position();
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
            throw parse_error("umatched \"{\"", begin, state.file());
        else
        {
            state.pop_front();
            return symbol{std::move(result), source_range{begin, state.position(), state.file()}};
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
        source_position begin = state.position();

        symbol::list result;
        do
        {
            state.pop_front(); // the first time, this will pop '[', afterwards ','
            whitespace(state);
            source_position before_nodes = state.position();
            
            symbol::list current_list = parse_nodes(state);
            result.push_back(symbol{std::move(current_list), source_range{before_nodes, state.position(), state.file()}});
        } while(!state.empty() && state.front() == ',');
        
        if(state.empty() || state.front() != ']')
            throw parse_error("unmatched \"[\"", begin, state.file());
        else
        {
            state.pop_front();
            return symbol{std::move(result), source_range{begin, state.position(), state.file()}};
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
        source_position begin = state.position();
        state.pop_front();

        whitespace(state);
        source_position first_begin = state.position();
        symbol::list first_list = parse_nodes(state);

        if(state.empty())
            throw parse_error("unmatched \"(\"", begin, state.file());
        else if(state.front() == ')')
        {
            state.pop_front();
            return symbol{std::move(first_list), source_range{begin, state.position(), state.file()}};
        }
        else if(state.front() == ',')
        {
            symbol::list result;
            result.push_back(symbol{std::move(first_list), source_range{first_begin, state.position(), state.file()}});
            do
            {
                state.pop_front();
                whitespace(state);
                source_position before = state.position();
                symbol::list current_list = parse_nodes(state);
                result.push_back(symbol{std::move(current_list), source_range{before, state.position(), state.file()}});
            } while(!state.empty() && state.front() == ',');
            
            if(state.empty() || state.front() != ')')
                throw parse_error("unmatched \"(\"", begin, state.file());
            else
            {
                state.pop_front();
                return symbol{std::move(result), source_range{begin, state.position(), state.file()}};
            }
        }
        else
            throw parse_error("unmatched \"(\"", begin, state.file());
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

template <class State>
symbol::list parse_file(State& state)
{
    symbol::list result;
    whitespace(state);
    boost::optional<symbol> next_list;
    while( (next_list = parse_semicolon_list(state)) )
    {
        result.push_back(*next_list);
        whitespace(state);
    }
    if(!state.empty())
        throw parse_error("invalid token after input", state.position(), state.file());
    else
        return result;
}

#endif

