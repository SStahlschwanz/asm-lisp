#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE compile_unit
#include <boost/test/unit_test.hpp>

#include "state_utils.hpp"
#include "../src/compile_unit.hpp"

#include <boost/filesystem.hpp>

using std::vector;
using std::pair;

using boost::filesystem::path;
using boost::filesystem::current_path;

using namespace symbol_shortcuts;

BOOST_AUTO_TEST_CASE(read_files_test)
{
    vector<path> paths = {"test-res/a.al", "test-res/b.al"};
    vector<pair<list, module_header>> files = read_files(paths, context);
    BOOST_CHECK_EQUAL(files.size(), 2);
    
    const ref export_st{"export"_id};
    const ref import{"import"_id};
    const ref from{"from"_id};
    const ref def{"def"_id};
    const ref a{"a"_id};
    const ref b{"b"_id};
    const ref c{"c"_id};
    const ref d{"d"_id};

    const list a_syntax_tree = list{
        list{export_st, a, b , c},
        list{def, a, list{}},
        list{def, b, list{}},
        list{def, c, a}
    };
    const list b_syntax_tree = list{
        list{import, list{a, b , c}, from, a},
        list{export_st, d},
        list{def, d, c}
    };

    BOOST_CHECK(a_syntax_tree == files[0].first);
    BOOST_CHECK(b_syntax_tree == files[1].first);
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
