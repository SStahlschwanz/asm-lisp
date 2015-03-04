#ifndef PARSE_HPP_
#define PARSE_HPP_

#include "symbol.hpp"
#include "parse_literal.hpp"
#include "parse_reference.hpp"
#include "whitespace.hpp"
#include "error/parse_error.hpp"

#include <boost/optional.hpp>

template<class State>
boost::optional<any_symbol> parse_node(State& state);

// parse 
//     {node, whitespace}
// and return list of nodes
template<class State>
list_symbol parse_nodes(State& state)
{
    list_symbol result;

    while(boost::optional<any_symbol> parse_result = parse_node(state))
    {
        result.push_back(std::move(*parse_result));
        whitespace(state);
    }
    return result;
}

// parse
//     {node, whitespace}, ";"
// and return list of nodes
template<class State>
boost::optional<list_symbol> parse_semicolon_list(State& state)
{
    using namespace parse_error;
    if(state.empty())
        return boost::none;
    else if(state.front() == ';')
    {
        // TODO: is this clause really necessary?
        file_position begin = state.position();
        state.pop_front();
        list_symbol result;
        result.source(file_source{begin, state.position(), state.file()});
        return result;
    }
    else
    {
        file_position begin = state.position();

        list_symbol result = parse_nodes(state);
        if(state.empty() || state.front() != ';')
        {
            // TODO: un-nest if clauses
            if(result.empty())
                return boost::none; // nothing was parsed -> this is not a semicolon list
            else
                fatal<id("unterminated_semicolon_list")>(code_location{state.position(), state.file()});
        }
        else
        {
            state.pop_front();
            result.source(file_source{begin, state.position(), state.file()});
            return result;
        }
    }
}

// parse
//     "{", {whitespace, semicolon_list}, "}"
// and return list of list of nodes
template<class State>
boost::optional<list_symbol> parse_curly_list(State& state)
{
    using namespace parse_error;
    if(state.empty() || state.front() != '{')
        return boost::none;
    else
    {
        file_position begin = state.position();
        state.pop_front();
        list_symbol result;
        boost::optional<list_symbol> semicolon_list;
        whitespace(state);
        while( (semicolon_list = parse_semicolon_list(state)) )
        {
            result.push_back(std::move(*semicolon_list));
            whitespace(state);
        }
        
        if(state.empty() || state.front() != '}')
            fatal<id("unmatched_curly_brace")>(code_location{begin, state.file()});
        else
        {
            state.pop_front();
            result.source(file_source{begin, state.position(), state.file()});
            return result;
        }
    }
}

// parse
//     "[", whitespace, nodes, {",", whitespace, nodes}, "]"
// and return list of list of nodes
template<class State>
boost::optional<list_symbol> parse_square_list(State& state)
{
    using namespace parse_error;
    if(state.empty() || state.front() != '[')
        return boost::none;
    else
    {
        file_position begin = state.position();

        list_symbol result;
        do
        {
            state.pop_front(); // the first time, this will pop '[', afterwards ','
            whitespace(state);
            file_position before_nodes = state.position();
            
            list_symbol current_list = parse_nodes(state);
            current_list.source(file_source{before_nodes, state.position(), state.file()});
            result.push_back(std::move(current_list));
        } while(!state.empty() && state.front() == ',');
        
        if(state.empty() || state.front() != ']')
            fatal<id("unmatched_square_brace")>(code_location{begin, state.file()});
        else
        {
            state.pop_front();
            result.source(file_source{begin, state.position(), state.file()});
            return result;
        }
    }
}

// parse round_list or round_comma_list
template <class State>
boost::optional<list_symbol> parse_round_list(State& state)
{
    using namespace parse_error;
    if(state.empty() || state.front() != '(')
        return boost::none;
    else
    {
        file_position begin = state.position();
        state.pop_front();

        whitespace(state);
        file_position first_begin = state.position();
        list_symbol first_list = parse_nodes(state);

        if(state.empty())
            fatal<id("unmatched_round_brace")>(code_location{begin, state.file()});
        else if(state.front() == ')')
        {
            state.pop_front();
            first_list.source(file_source{begin, state.position(), state.file()});
            return first_list;
        }
        else if(state.front() == ',')
        {
            list_symbol result;
            first_list.source(file_source{first_begin, state.position(), state.file()});
            result.push_back(std::move(first_list));
            do
            {
                state.pop_front();
                whitespace(state);
                file_position before = state.position();
                list_symbol current_list = parse_nodes(state);
                current_list.source(file_source{before, state.position(), state.file()});
                result.push_back(std::move(current_list));
            } while(!state.empty() && state.front() == ',');
            
            if(state.empty() || state.front() != ')')
                fatal<id("unmatched_round_brace")>(code_location{begin, state.file()});
            else
            {
                state.pop_front();
                result.source(file_source{begin, state.position(), state.file()});
                return result;
            }
        }
        else
            fatal<id("unmatched_round_brace")>(code_location{begin, state.file()});
    }
}

template <class State>
boost::optional<any_symbol> parse_node(State& state)
{
    boost::optional<any_symbol> result;
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
list_symbol parse_file(State& state)
{
    using namespace parse_error;
    list_symbol result;
    file_position begin = state.position();
    whitespace(state);
    boost::optional<list_symbol> next_list;
    while( (next_list = parse_semicolon_list(state)) )
    {
        result.push_back(*next_list);
        whitespace(state);
    }

    if(!state.empty())
        fatal<id("invalid_character")>(code_location{state.position(), state.file()});

    result.source(file_source{begin, state.position(), state.file()});
    return result;
}

#endif

