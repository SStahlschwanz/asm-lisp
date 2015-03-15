#ifndef PARSE_HPP_
#define PARSE_HPP_

#include "node.hpp"
#include "parse_literal.hpp"
#include "parse_reference.hpp"
#include "whitespace.hpp"
#include "error/parse_error.hpp"

#include <vector>

template<class State>
node* parse_node(State& state);

// parse 
//     {node, whitespace}
// and return list of nodes
template<class State>
std::vector<node*> parse_nodes(State& state)
{
    std::vector<node*> vec;

    while(node* parse_result = parse_node(state))
    {
        vec.push_back(std::move(parse_result));
        whitespace(state);
    }
    return vec;
}

// parse
//     {node, whitespace}, ";"
// and return list of nodes
template<class State>
list_node* parse_semicolon_list(State& state)
{
    using namespace parse_error;
    if(state.empty())
        return nullptr;

    file_position begin = state.position();
    std::vector<node*> vec = parse_nodes(state);

    if(vec.empty())
    {
        if(!state.empty() && state.front() == ';')
        {
            state.pop_front();
            list_node& result = state.graph().create_list({});
            result.source(file_source({begin, state.position(), state.file()}));
            return &result;
        }
        
        return nullptr;
    }

    if(state.empty() || state.front() != ';')
        fatal<id("unterminated_semicolon_list")>(code_location{state.position(), state.file()});
        
    state.pop_front();
    list_node& result = state.graph().create_list(std::move(vec));
    result.source(file_source{begin, state.position(), state.file()});
    return &result;
}

// parse
//     "{", {whitespace, semicolon_list}, "}"
// and return list of list of nodes
template<class State>
list_node* parse_curly_list(State& state)
{
    using namespace parse_error;
    if(state.empty() || state.front() != '{')
        return nullptr;
        
    file_position begin = state.position();
    state.pop_front();
    std::vector<node*> top_level_nodes;

    whitespace(state);
    while(node* semicolon_list = parse_semicolon_list(state))
    {
        top_level_nodes.push_back(semicolon_list);
        whitespace(state);
    }

    if(state.empty() || state.front() != '}')
        fatal<id("unmatched_curly_brace")>(code_location{begin, state.file()});
    
    state.pop_front();
    list_node& result = state.graph().create_list(std::move(top_level_nodes));
    result.source(file_source{begin, state.position(), state.file()});
    return &result;
}

template<class State>
list_node* parse_round_list(State& state)
{
    using namespace parse_error;

    if(state.empty() || state.front() != '(')
        return nullptr;
    
    file_position begin = state.position();
    state.pop_front();
    whitespace(state);
    std::vector<node*> vec = parse_nodes(state);
    
    if(state.empty() || state.front() != ')')
        fatal<id("unmatched_round_brace")>(code_location{begin, state.file()});

    state.pop_front();

    list_node& result = state.graph().create_list(std::move(vec));
    result.source(file_source{begin, state.position(), state.file()});
    return &result;
}


template <class State>
node* parse_node(State& state)
{
    if(node* result = parse_literal(state))
        return result;
    if(node* result = parse_reference(state))
        return result;
    if(node* result = parse_curly_list(state))
        return result;
    if(node* result = parse_round_list(state))
        return result;

    return nullptr;
}

template <class State>
list_node& parse_file(State& state)
{
    using namespace parse_error;

    std::vector<node*> top_level_lists;

    file_position begin = state.position();
    whitespace(state);
    while(node* next_list = parse_semicolon_list(state))
    {
        top_level_lists.push_back(next_list);
        whitespace(state);
    }

    list_node& result = state.graph().create_list(std::move(top_level_lists));

    if(!state.empty())
        fatal<id("invalid_character")>(code_location{state.position(), state.file()});

    result.source(file_source{begin, state.position(), state.file()});
    return result;
}

#endif

