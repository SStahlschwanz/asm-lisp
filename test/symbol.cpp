#include <symbol.hpp>


using namespace symbol_building;

int main()
{
    {
        symbol a = lit("asdf");
        symbol b = lit("asdf");
        assert(a == b);
    }
    {
        symbol a = list(lit("asdf"), ref("sdf"));
        symbol b = list(lit("asdf"), ref("sdf"));
        assert(a == b);
    }
    {
        symbol a = list(lit("ff"), ref("fff"), list());
        symbol b = list(lit("asdff"), ref("fff"), list());
        assert(a != b);
    }
    {
        symbol a = list(lit("asdff"), ref("fff"), list());
        symbol b = list(lit("asdff"), ref("fff"), list());
        
        symbol c = ref("ff", &a);
        symbol d = ref("ff", &b);
        assert(c == d);
        
        a = list(lit("asdff"), ref(""), list());
        assert(c != d);

        c = ref("ff", &b);
        assert(c == d);

        c = ref("ffsdf", &b);
        assert(c != d);

        c = ref("ff");
        assert(c != d);
    }
}
