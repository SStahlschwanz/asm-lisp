#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse

#include <boost/test/unit_test.hpp>

#include "../src/indexed_symbol.hpp"

#include "state_utils.hpp"

using std::vector;
using std::unique_ptr;

using boost::get;

using namespace symbol_shortcuts;

BOOST_AUTO_TEST_CASE(list_test)
{
    list_symbol l{list_symbol{}};
    vector<indexed_symbol> indexed_symbols = to_indexed_symbol(l.begin(), l.end());
    
    BOOST_CHECK_EQUAL(indexed_symbols.size(), 2);
    BOOST_CHECK_EQUAL(indexed_symbols[0].first, 1);
    BOOST_CHECK_EQUAL(indexed_symbols[1].first, 2);
    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0].second).vec.size(), 1);
    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0].second).vec[0], 2);

    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[1].second).vec.size(), 0);
}

BOOST_AUTO_TEST_CASE(ref_test)
{
    ref_symbol r1{"a"_id};
    ref_symbol r2{"b"_id, &r1};
    list_symbol l{r2};

    vector<indexed_symbol> indexed_symbols = to_indexed_symbol(l.begin(), l.end());

    BOOST_CHECK_EQUAL(indexed_symbols.size(), 3);
    BOOST_CHECK_EQUAL(indexed_symbols[0].first, 1);
    BOOST_CHECK_EQUAL(indexed_symbols[1].first, 2);
    BOOST_CHECK_EQUAL(indexed_symbols[2].first, 3);

    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0].second).vec.size(), 1);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[1].second).identifier, "b"_id);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[1].second).refered_index, 3);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[2].second).identifier, "a"_id);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[2].second).refered_index, 0);
}

BOOST_AUTO_TEST_CASE(lit_test)
{
    lit_symbol literal{"a"};
    list_symbol l{literal};

    vector<indexed_symbol> indexed_symbols = to_indexed_symbol(l.begin(), l.end());

    BOOST_CHECK_EQUAL(indexed_symbols.size(), 2);
    BOOST_CHECK_EQUAL(indexed_symbols[0].first, 1);
    BOOST_CHECK_EQUAL(indexed_symbols[1].first, 2);

    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0].second).vec.size(), 1);
    BOOST_CHECK_EQUAL(get<indexed_lit>(indexed_symbols[1].second).str, "a");
}

BOOST_AUTO_TEST_CASE(forth_back_test)
{
    ref r{"123"_id};
    list l{lit{"abc"}, list{ref{"345"_id, &r}, list{}}};

    auto p = to_symbol(1, to_indexed_symbol(l.begin(), l.end()));

    any_symbol& s = p.first;
    vector<unique_ptr<any_symbol>>& symbol_store = p.second;

    BOOST_CHECK(structurally_equal(s, l));
    BOOST_CHECK_EQUAL(symbol_store.size(), 1);
}
