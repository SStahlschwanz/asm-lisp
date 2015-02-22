#define BOOST_TEST_MODULE module
#include <boost/test/included/unit_test.hpp>

#include "../src/module.hpp"

#include "state_utils.hpp"

using std::unordered_map;
using std::string;
using std::vector;

typedef lit_symbol lit;
typedef ref_symbol ref;
typedef list_symbol list;

const ref export_tok{"export"};
const ref import_tok{"import"};
const ref from_tok{"from"};

const list_symbol mod1_tree = list{
    list{export_tok, ref{"a"}, ref{"b"}, ref{"c"}}
};
const list_symbol mod2_tree = list{
    list{export_tok, ref{"x"}, ref{"y"}, ref{"z"}},
    list{import_tok, list{ref{"a"}, ref{"b"}, ref{"c"}}, from_tok, ref{"mod1"}}
};

BOOST_AUTO_TEST_CASE(module_test)
{
    module_header mod1_header = read_module_header(mod1_tree);
    module_header mod2_header = read_module_header(mod2_tree);

    unordered_map<string, vector<symbol_source>> mod1_imported_modules =
            imported_modules(mod1_header);
    BOOST_CHECK(mod1_imported_modules.empty());

    unordered_map<string, vector<symbol_source>> mod2_imported_modules =
            imported_modules(mod2_header);
    BOOST_CHECK(mod2_imported_modules.size() == 1);
    BOOST_CHECK(mod2_imported_modules.count("mod1"));
}
