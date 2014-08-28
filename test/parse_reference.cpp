#include <parse_reference.hpp>
#include <parse_reference.hpp>

#include <parse_state.hpp>

#include <string>

using namespace std;

int main()
{
    typedef parse_state<string::iterator> state;
    {
        string str = "abc9";
        state state(str.begin(), str.end());

        symbol result = *parse_reference(state);
        assert(boost::get<symbol::reference>(result.content).identifier == "abc");
        assert(remaining(state) == "9");
    }
    {
        string str = "abc";
        state state(str.begin(), str.end());

        symbol result = *parse_reference(state);
        assert(boost::get<symbol::reference>(result.content).identifier == "abc");
        assert(state.empty());
    }
    {
        string str = "9abc";
        state state(str.begin(), str.end());
        
        assert(!parse_reference(state));
        assert(remaining(state) == str);
    }
    {
        string str = "{abc";
        state state(str.begin(), str.end());

        assert(!parse_reference(state));
        assert(remaining(state) == str);
    }
}
