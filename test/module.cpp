#include <module.hpp>

#include <cassert>

using namespace std;
using namespace symbol_building;

int main()
{
    module mod = read_module("test-res/module1.al");
    symbol::list syntax_tree =
    {
        list(list()),
        list(lit("asdf"), sref("asdfa"), sref("f")),
        list(sref("import"), sref("fff")),
        list(sref("ff")),
        list(sref("ff")),
        list(sref("import"), sref("aa"), sref("bb")),
        list(sref("import"), sref("aa"))
    };
    assert(mod.syntax_tree == syntax_tree);

    vector<string> required_modules = {"fff", "aa", "bb"};
    std::sort(required_modules.begin(), required_modules.end());
    
    assert(mod.required_modules == required_modules);
}
