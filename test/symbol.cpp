#include "symbol_building.hpp"


int main()
{
    {
        symbol a = lit("asdf");
        symbol b = lit("asdf");
        assert(a == b);
    }
    {
        symbol a = list(lit("asdf"), sref("sdf"));
        symbol b = list(lit("asdf"), sref("sdf"));
        assert(a == b);
    }
    {
        symbol a = list(lit("ff"), sref("fff"), list());
        symbol b = list(lit("asdff"), sref("fff"), list());
        assert(a != b);
    }
    {
        symbol a = list(lit("asdff"), sref("fff"), list());
        symbol b = list(lit("asdff"), sref("fff"), list());
        
        symbol c = sref("ff", &a);
        symbol d = sref("ff", &b);
        assert(c == d);
        
        a = list(lit("asdff"), sref(""), list());
        assert(c != d);

        c = sref("ff", &b);
        assert(c == d);

        c = sref("ffsdf", &b);
        assert(c != d);

        c = sref("ff");
        assert(c != d);
    }
}
