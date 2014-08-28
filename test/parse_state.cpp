#include <parse_state.hpp>
#include <parse_state.hpp>


int main()
{
    std::string str = "as\ndf";
    parse_state<std::string::iterator> state(str.begin(), str.end());

    
    assert(!str.empty());
    assert(state.location().pos == 0);
    assert(state.location().line == 0);
    assert(state.front() == 'a');
    
    state.pop_front();
    
    assert(!state.empty());
    assert(state.location().pos == 1);
    assert(state.location().line == 0);
    assert(state.front() == 's');

    state.pop_front();
    assert(!state.empty());
    assert(state.location().pos == 2);
    assert(state.location().line == 0);
    assert(state.front() == '\n');

    state.pop_front();
    assert(!state.empty());
    assert(state.location().pos == 0);
    assert(state.location().line == 1);
    assert(state.front() == 'd');

    state.pop_front();
    assert(!state.empty());
    assert(state.location().pos == 1);
    assert(state.location().line == 1);
    assert(state.front() == 'f');
    
    state.pop_front();
    assert(state.empty());
}
