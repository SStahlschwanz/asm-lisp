#ifndef WHITESPACE_HPP_
#define WHITESPACE_HPP_

namespace whitespace_detail
{

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

}

template <class State>
bool whitespace(State& state)
{
    using namespace whitespace_detail;

    if(state.empty() || !is_whitespace(state.front()))
        return false;
    else
    {
        state.pop_front();
        while(!state.empty() && is_whitespace(state.front()))
            state.pop_front();
        return true;
    }
}

#endif

