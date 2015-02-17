#include "../src/module.hpp"

#include "symbol_building.hpp"

#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

using namespace std;

stringstream source1(
R"(
{};

"asdf" asdfa f;
import fff;
ff;
ff;
import aa
bb;
import aa;
)");

symbol::list source1_syntax = 
{
    list(list()),
    list(lit("asdf"), sref("asdfa"), sref("f")),
    list(sref("import"), sref("fff")),
    list(sref("ff")),
    list(sref("ff")),
    list(sref("import"), sref("aa"), sref("bb")),
    list(sref("import"), sref("aa"))
};

vector<string> source1_imports = {"fff", "aa", "bb"};


stringstream source2(
R"(def a {};
def b "asdf";
def c x;
def d b;
)");

symbol::list source2_syntax =
{
    list(sref("def"), sref("a"), list()),
    list(sref("def"), sref("b"), lit("asdf")),
    list(sref("def"), sref("c"), sref("x")),
    list(sref("def"), sref("d"), sref("b"))
};

vector<string> source2_imports = {};

module verify(stringstream& source, const symbol::list& syntax_tree, vector<string> imports)
{
    module mod = read_module(source);
    assert(mod.syntax_tree == syntax_tree);
    sort(imports.begin(), imports.end());
    assert(mod.required_modules == imports);
    return mod;
}

int main()
{
    verify(source1, source1_syntax, source1_imports);
    module m2 = verify(source2, source2_syntax, source2_imports);
    dispatch_and_eval(m2, {});
    symbol a_def = list() ;
    symbol b_def = lit("asdf");
    symbol c_def = sref("x");
    symbol d_def = sref("b", &b_def);
    assert(m2.defined_symbols.size() == 4);
    assert(a_def == m2.defined_symbols["a"]);
    assert(b_def == m2.defined_symbols["b"]);
    assert(c_def == m2.defined_symbols["c"]);
    assert(d_def == m2.defined_symbols["d"]);
}
