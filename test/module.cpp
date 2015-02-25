#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include "../src/module.hpp"

#include "state_utils.hpp"

using std::unordered_map;
using std::string;
using std::vector;

typedef lit_symbol lit;
typedef ref_symbol ref;
typedef list_symbol list;

const ref export_st{"export"};
const ref import{"import"};
const ref from{"from"};
const ref def{"def"};

const ref mod1{"mod1"};
const ref mod2{"mod2"};

const ref a{"a"};
const ref b{"b"};
const ref c{"c"};

const ref x{"x"};
const ref y{"y"};
const ref z{"z"};

const list_symbol mod1_tree = list
{
    list{export_st, a, b, c},
    list{def, a, list{}},
    list{def, b, list{lit{"bb"}}},
    list{def, c, list{}}
};
const export_statement mod1_export1{mod1_tree[0].cast<list>()};

const list_symbol mod2_tree = list
{
    list{import, list{a, c}, from, mod1},
    list{export_st, x, y},
    list{import, list{b, c}, from, mod1},
    list{export_st, y, z},
    list{def, x, a},
    list{def, y, list{b}},
    list{def, z, list{}}
};
const import_statement mod2_import1{
        mod2_tree[0].cast<list>(),
        mod2_tree[0].cast<list>()[3].cast<ref>(),
        mod2_tree[0].cast<list>()[1].cast<list>()};
const import_statement mod2_import2{
        mod2_tree[2].cast<list>(),
        mod2_tree[2].cast<list>()[3].cast<ref>(),
        mod2_tree[2].cast<list>()[1].cast<list>()};
const export_statement mod2_export1{mod2_tree[1].cast<list>()};
const export_statement mod2_export2{mod2_tree[3].cast<list>()};


bool operator==(const export_statement& lhs, const export_statement& rhs)
{
    return lhs.statement == rhs.statement;
}
bool operator==(const import_statement& lhs, const import_statement& rhs)
{
    return lhs.statement == rhs.statement && 
            lhs.imported_module == rhs.imported_module &&
            lhs.import_list == rhs.import_list;
}

BOOST_AUTO_TEST_CASE(header_test)
{
    module_header mod1_header = read_module_header(mod1_tree);
    
    BOOST_CHECK(mod1_header.imports.empty());
    
    BOOST_CHECK_EQUAL(mod1_header.exports.size(), 1);
    BOOST_CHECK(mod1_header.exports[0] == mod1_export1);
    

    module_header mod2_header = read_module_header(mod2_tree);

    BOOST_CHECK_EQUAL(mod2_header.imports.size(), 2);
    BOOST_CHECK(mod2_header.imports[0] == mod2_import1);
    BOOST_CHECK(mod2_header.imports[1] == mod2_import2);
    
    BOOST_CHECK_EQUAL(mod2_header.exports.size(), 2);
    BOOST_CHECK(mod2_header.exports[0] == mod2_export1);
    BOOST_CHECK(mod2_header.exports[1] == mod2_export2);
    

    unordered_map<string, vector<symbol_source>> mod1_imported_modules =
            imported_modules(mod1_header);
    BOOST_CHECK(mod1_imported_modules.empty());

    unordered_map<string, vector<symbol_source>> mod2_imported_modules =
            imported_modules(mod2_header);
    BOOST_CHECK(mod2_imported_modules.size() == 1);
    BOOST_CHECK(mod2_imported_modules.count("mod1"));
}

BOOST_AUTO_TEST_CASE(simple_definition_test)
{
    compilation_context context;
    
    module_header mod1_header = {{}, {mod1_export1}};
    module_header mod2_header =
            {{mod2_import1, mod2_import2}, {mod2_export1, mod2_export2}};
    
    unordered_map<string, module> module_map;
    module_map["mod1"] = evaluate_module(mod1_tree, mod1_header, module_map, context);
    auto& exports1 = module_map["mod1"].exports;
    BOOST_CHECK(exports1.size() == 3);
    BOOST_CHECK(exports1.count("a") && exports1.count("b") && exports1.count("c"));
    BOOST_CHECK(*exports1["a"] == list{});
    BOOST_CHECK(*exports1["b"] == list{lit{"bb"}});
    BOOST_CHECK(*exports1["c"] == list{});

    module_map["mod2"] = evaluate_module(mod2_tree, mod2_header, module_map, context);
    auto& exports2 = module_map["mod2"].exports;
    BOOST_CHECK(exports2.size() == 3);
    BOOST_CHECK(exports2.count("x") && exports2.count("y") && exports2.count("z"));
    
    // need "((" because of BOOST_CHECK internals (bug?)
    BOOST_CHECK((*exports2["x"] == ref{"a", exports1["a"]}));
    BOOST_CHECK((*exports2["y"] == list{ref{"b", exports1["b"]}}));
}
