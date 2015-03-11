#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse

#include <boost/test/unit_test.hpp>

#include "../src/indexed_symbol.hpp"

#include "state_utils.hpp"

using std::vector;
using std::unique_ptr;
using std::pair;
using std::make_shared;
using std::move;

using boost::get;

using llvm::Function;

using namespace symbol_shortcuts;

BOOST_AUTO_TEST_CASE(list_test)
{
    list_symbol l{list_symbol{}};
    vector<indexed_symbol> indexed_symbols = to_indexed_symbol(l.begin(), l.end());
    
    BOOST_CHECK_EQUAL(indexed_symbols.size(), 2);
    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0]).vec.size(), 1);
    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0]).vec[0], 2);

    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[1]).vec.size(), 0);
}

BOOST_AUTO_TEST_CASE(ref_test)
{
    ref_symbol r1{"a"_id};
    ref_symbol r2{"b"_id, &r1};
    list_symbol l{r2};

    vector<indexed_symbol> indexed_symbols = to_indexed_symbol(l.begin(), l.end());

    BOOST_CHECK_EQUAL(indexed_symbols.size(), 3);

    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0]).vec.size(), 1);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[1]).identifier, "b"_id);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[1]).refered_index, 3);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[2]).identifier, "a"_id);
    BOOST_CHECK_EQUAL(get<indexed_ref>(indexed_symbols[2]).refered_index, 0);
}

BOOST_AUTO_TEST_CASE(lit_test)
{
    lit_symbol literal{"a"};
    list_symbol l{literal};

    vector<indexed_symbol> indexed_symbols = to_indexed_symbol(l.begin(), l.end());

    BOOST_CHECK_EQUAL(indexed_symbols.size(), 2);

    BOOST_CHECK_EQUAL(get<indexed_list>(indexed_symbols[0]).vec.size(), 1);
    BOOST_CHECK_EQUAL(get<indexed_lit>(indexed_symbols[1]).str, "a");
}


BOOST_AUTO_TEST_CASE(forth_back_test)
{
    auto rt_f = reinterpret_cast<Function*>(345234);
    auto ct_f = reinterpret_cast<Function*>(98989);
    auto macro_func = [](list_symbol::const_iterator, list_symbol::const_iterator)
    {
        assert(false);
        // this function is not called, so this is ok
        return move(*static_cast<pair<any_symbol, vector<unique_ptr<any_symbol>>>*>(nullptr));
    };
    ref r{"123"_id};
    list l1 = 
    {
        lit{"abc"},
        list{ref{"345"_id, &r}, list{}},
        id_symbol{1234},
        proc_symbol{ct_f, rt_f},
        macro_symbol{make_shared<macro_symbol::macro_function>(macro_func)}
    };

    auto p1 = to_symbol(1, to_indexed_symbol(l1.begin(), l1.end()));

    const symbol& s1 = p1.first;
    vector<unique_ptr<any_symbol>>& symbol_store1 = p1.second;

    BOOST_CHECK(structurally_equal(s1, l1));
    BOOST_CHECK_EQUAL(symbol_store1.size(), 1);

    list l2{lit{}};

    auto p2 = to_symbol(1, to_indexed_symbol(l2.begin(), l2.end()));

    const symbol& s2 = p2.first;
    vector<unique_ptr<any_symbol>>& symbol_store2 = p2.second;

    BOOST_CHECK(structurally_equal(s2, l2));
    BOOST_CHECK_EQUAL(symbol_store2.size(), 0);
}
