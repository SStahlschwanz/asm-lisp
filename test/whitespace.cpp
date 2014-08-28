#include <whitespace.hpp>
#include <whitespace.hpp>

#include <parse_state.hpp>

#include <cassert>
#include <string>

using namespace std;

int main()
{
    typedef parse_state<string::iterator> state;
    {
        string str = " \n\t";
        state state(begin(str), end(str));
        assert(whitespace(state));
        assert(state.empty());
    }
    {
        string str = "f \n\t";
        state state(begin(str), end(str));
        assert(!whitespace(state));
        assert(remaining(state) == str);
    }
    {
        string str = "  f ";
        state state(begin(str), end(str));
        assert(whitespace(state));
        assert(remaining(state) == "f ");
    }
}
