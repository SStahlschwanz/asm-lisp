#include <parse_literal.hpp>
#include <parse_literal.hpp>

#include <parse_state.hpp>

#include <cassert>

using namespace std;

int main()
{
    typedef parse_state<string::iterator> state;
    
    {
        string str = "\"abc\"";
        state state(begin(str), end(str));
        symbol symbol = *parse_literal(state);
        assert(boost::get<symbol::literal>(symbol.content) == "abc");
        assert(state.empty());
    }
    {
        string str = "912fas31";
        state state(begin(str), end(str));
        symbol symbol = *parse_literal(state);
        assert(boost::get<symbol::literal>(symbol.content) == "912");
        assert(remaining(state) == "fas31");
    }
    {
        string str = "\"fsd";
        state state(begin(str), end(str));
        try
        {
            parse_literal(state);
            assert(false);
        }
        catch(const parse_error& err)
        {}
    }
    {
        string str = "f\"91";
        state state(begin(str), end(str));
        assert(!parse_literal(state));
        assert(remaining(state) == str);
    }
}
