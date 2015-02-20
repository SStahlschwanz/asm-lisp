#define BOOST_TEST_MODULE module
#include <boost/test/included/unit_test.hpp>

#include "../src/module.hpp"


#include "state_utils.hpp"
#include "symbol_building.hpp"

#include <vector>
#include <string>
#include <utility>
#include <set>

using std::vector;
using std::string;
using std::pair;
using std::set;
using std::unordered_map;

vector<pair<string, string>> imports_to_strings(const vector<module::import_entry>& imports)
{
    vector<pair<string, string>> result;
    for(const module::import_entry& entry : imports)
        result.push_back({entry.module, entry.identifier});
    return result;

}

set<string> exports_to_set(const unordered_map<string, symbol>& exports)
{
    set<string> result;
    for(const auto& v : exports)
        result.insert(v.first);
    return result;
}



const state src_1 = make_state(
R"(
import asdf (ggg asdf asdf);
export ff ss;
def ff (());
def ss {};
)");

BOOST_AUTO_TEST_CASE(import_output_test)
{
    state s = src_1;
    module m = module::read(s);
    auto got_imports = imports_to_strings(m.imports);
    vector<pair<string, string>> expected_imports{{"asdf", "asdf"}, {"asdf", "ggg"}};
    BOOST_CHECK(got_imports == expected_imports);

    auto got_exports = exports_to_set(m.exports);
    set<string> expected_exports = {"ff", "ss"};
    BOOST_CHECK(got_exports == expected_exports);
}

const state src_2 = make_state(
R"(
export ggg asdf;

def ggg ();
def asdf ();
)");
BOOST_AUTO_TEST_CASE(define_test)
//int main(int argc, char** argv)
{
    state s1 = src_1;
    state s2 = src_2;

    module m1 = module::read(s1);
    module m2 = module::read(s2);
    
    unordered_map<string, const module::export_table*> export_tables;
    m2.evaluate_exports(export_tables);
    unordered_map<string, symbol> expected_m2_exports =
    {
        {"gg", list()},
        {"asdf", list()}
    };
    BOOST_CHECK(m2.exports == expected_m2_exports);
    export_tables["asdf"] = &m2.exports;
    
    m1.evaluate_exports(export_tables);
}
