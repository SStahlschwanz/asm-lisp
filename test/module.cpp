#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE module
#include <boost/test/unit_test.hpp>

#include "../src/module.hpp"

#include "state_utils.hpp"
#include "context.hpp"

using std::unordered_map;
using std::string;
using std::vector;
using std::pair;
using std::shared_ptr;
using std::make_shared;
using std::move;

ref_node& export_st = ref("export");
ref_node& import = ref{"import"};
ref_node& from = ref{"from"};
ref_node& def = ref{"def"};

lit_node& mod1 = lit{"mod1"};
lit_node& mod2 = lit{"mod2"};
lit_node& injected = lit{"injected"};

ref_node& a = ref{"a"};
ref_node& b = ref{"b"};
ref_node& c = ref{"c"};
ref_node& pf = ref{"pf"};

ref_node& x = ref{"x"};
ref_node& y = ref{"y"};
ref_node& z = ref{"z"};

list_node& mod1_tree = list
{
    list{export_st, a, b, c},
    list{import, list{pf}, from, injected},
    list{def, a, pf},
    list{def, b, pf, list{lit{"bb"}}},
    list{def, c, pf, list{}}
};
import_statement mod1_import1
{
        mod1_tree[1].cast<list_node>(),
        mod1_tree[1].cast<list_node>()[3].cast<lit_node>(),
        mod1_tree[1].cast<list_node>()[1].cast<list_node>()
};
export_statement mod1_export1{mod1_tree[0].cast<list_node>()};

list_node& mod2_tree = list
{
    list{import, list{a, c}, from, mod1},
    list{import, list{pf}, from, injected},
    list{export_st, x, y},
    list{import, list{b, c}, from, mod1},
    list{export_st, y, z},
    list{def, x, pf, a},
    list{def, y, pf, list{b}},
    list{def, z, pf, list{}}
};
import_statement mod2_import1
{
        mod2_tree[0].cast<list_node>(),
        mod2_tree[0].cast<list_node>()[3].cast<lit_node>(),
        mod2_tree[0].cast<list_node>()[1].cast<list_node>()
};
import_statement mod2_import2
{
        mod2_tree[1].cast<list_node>(),
        mod2_tree[1].cast<list_node>()[3].cast<lit_node>(),
        mod2_tree[1].cast<list_node>()[1].cast<list_node>()
};
import_statement mod2_import3
{
        mod2_tree[3].cast<list_node>(),
        mod2_tree[3].cast<list_node>()[3].cast<lit_node>(),
        mod2_tree[3].cast<list_node>()[1].cast<list_node>()
};
export_statement mod2_export1{mod2_tree[2].cast<list_node>()};
export_statement mod2_export2{mod2_tree[4].cast<list_node>()};


bool operator==(const export_statement& lhs, const export_statement& rhs)
{
    return structurally_equal(lhs.statement, rhs.statement);
}
bool operator==(const import_statement& lhs, const import_statement& rhs)
{
    return structurally_equal(lhs.statement, rhs.statement) &&
        structurally_equal(lhs.statement, rhs.statement) &&
        structurally_equal(lhs.import_list, rhs.import_list);
}

BOOST_AUTO_TEST_CASE(header_test)
{
    module_header mod1_header = read_module_header(mod1_tree);
    
    BOOST_CHECK_EQUAL(mod1_header.imports.size(), 1);
    
    BOOST_CHECK_EQUAL(mod1_header.exports.size(), 1);
    BOOST_CHECK(mod1_header.exports[0] == mod1_export1);
    

    module_header mod2_header = read_module_header(mod2_tree);

    BOOST_CHECK_EQUAL(mod2_header.imports.size(), 3);
    BOOST_CHECK(mod2_header.imports[0] == mod2_import1);
    BOOST_CHECK(mod2_header.imports[1] == mod2_import2);
    BOOST_CHECK(mod2_header.imports[2] == mod2_import3);
    
    BOOST_CHECK_EQUAL(mod2_header.exports.size(), 2);
    BOOST_CHECK(mod2_header.exports[0] == mod2_export1);
    BOOST_CHECK(mod2_header.exports[1] == mod2_export2);
    

    unordered_map<string, vector<node_source>> mod1_imported_modules = imported_modules(mod1_header);
    BOOST_CHECK_EQUAL(mod1_imported_modules.size(), 1);
    BOOST_CHECK(mod1_imported_modules.count("injected"));

    unordered_map<string, vector<node_source>> mod2_imported_modules = imported_modules(mod2_header);
    BOOST_CHECK(mod2_imported_modules.size() == 2);
    BOOST_CHECK(mod2_imported_modules.count("mod1"));
    BOOST_CHECK(mod2_imported_modules.count("injected"));
}

BOOST_AUTO_TEST_CASE(simple_definition_test)
{
    auto passthrough_function = [](node_range r) -> pair<node&, dynamic_graph>
    {
        dynamic_graph graph;
        auto node_ptrs = save<vector<node*>>(mapped(r, [](node& n)
        {
            return &n;
        }));
        list_node& l = graph.create_list(move(node_ptrs));
        return {l, move(graph)};
    };
    

    dynamic_graph module_graph;
    macro_node& macro = module_graph.create_macro();
    macro.function(std::make_shared<std::function<macro_node::macro>>(passthrough_function));
    module injected_module{move(module_graph), {{"pf", macro}}};

    unordered_map<string, module&> module_map;
    module_map.insert({"injected", injected_module});

    auto lookup_module = [&](const import_statement& st) -> module&
    {
        string mod_name = save<string>(rangeify(st.imported_module));
        BOOST_CHECK(module_map.count(mod_name));
        return module_map.at(mod_name);
    };

    module_header mod1_header = {{mod1_import1}, {mod1_export1}};
    module_header mod2_header = {{mod2_import1, mod2_import2, mod2_import3}, {mod2_export1, mod2_export2}};
    
    module mod1 = evaluate_module(mod1_tree, dynamic_graph{}, mod1_header, lookup_module);

    module_map.insert({"mod1", mod1});

    auto& exports1 = mod1.exports;
    BOOST_CHECK(exports1.size() == 3);
    BOOST_CHECK(exports1.count("a") && exports1.count("b") && exports1.count("c"));
    BOOST_CHECK(structurally_equal(exports1.at("a"), list{}));
    BOOST_CHECK(structurally_equal(exports1.at("b"), list{list{lit{"bb"}}}));
    BOOST_CHECK(structurally_equal(exports1.at("c"), list{list{}}));

    module mod2 = evaluate_module(mod2_tree, dynamic_graph{}, mod2_header, lookup_module);
    module_map.insert({"mod2", mod2});
    
    auto& exports2 = mod2.exports;
    BOOST_CHECK(exports2.size() == 3);
    BOOST_CHECK(exports2.count("x") && exports2.count("y") && exports2.count("z"));
}

