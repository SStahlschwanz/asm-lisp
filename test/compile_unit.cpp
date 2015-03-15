#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_unit
#include <boost/test/unit_test.hpp>

#include "state_utils.hpp"
#include "../src/compile_unit.hpp"

#include "context.hpp"

#include <boost/filesystem.hpp>

using std::vector;
using std::pair;

using boost::filesystem::path;
using boost::filesystem::current_path;


BOOST_AUTO_TEST_CASE(read_files_test)
{
    vector<path> paths = {"test-res/a/a.al", "test-res/b/b.al"};
    auto parsed_files = save<vector<parsed_file>>(mapped(enumerate(paths), unpacking(read_file)));
    BOOST_CHECK_EQUAL(parsed_files.size(), 2);
    
    ref_node& export_st = ref{"export"};
    ref_node& import = ref{"import"};
    ref_node& from = ref{"from"};
    ref_node& def = ref{"def"};
    ref_node& a = ref{"a"};
    ref_node& b = ref{"b"};
    ref_node& c = ref{"c"};
    ref_node& d = ref{"d"};

    list_node& a_syntax_tree = list{
        list{export_st, a, b, c},
        list{def, a, list{}},
        list{def, b, list{}},
        list{def, c, a}
    };
    list_node& b_syntax_tree = list{
        list{import, list{a, b , c}, from, lit{"../a/a"}},
        list{export_st, d},
        list{def, d, c}
    };

    BOOST_CHECK(structurally_equal(a_syntax_tree, parsed_files[0].syntax_tree));
    BOOST_CHECK(structurally_equal(b_syntax_tree, parsed_files[1].syntax_tree));
}

BOOST_AUTO_TEST_CASE(toposort_test)
{
    vector<vector<size_t>> graph1
    {
        {1},
        {}
    };
    vector<size_t> sorted1 = toposort(graph1);
    vector<size_t> expected1 = {1, 0};
    BOOST_CHECK(sorted1 == expected1);
    
    vector<vector<size_t>> graph2
    {
        {},
        {0}
    };
    vector<size_t> sorted2 = toposort(graph2);
    vector<size_t> expected2 = {0, 1};
    BOOST_CHECK(sorted2 == expected2);
}
