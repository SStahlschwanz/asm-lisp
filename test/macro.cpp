#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE macro
#include <boost/test/unit_test.hpp>

#include "../src/macro_execution.hpp"

#include "state_utils.hpp"
#include "function_building.hpp"


typedef size_t macro_function(size_t);


BOOST_AUTO_TEST_CASE(parameter_return_test)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    const list body =
    {
        list{block1, list
        {
            list{return_symbol, s}
        }}
    };
    
    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l1;
    auto p1 = execute_macro(func, l1.begin(), l1.end());
    BOOST_CHECK(structurally_equal(p1.first, l1));
    
    list l2{lit{"abc"}, ref{"123"_id, &l1}};
    auto p2 = execute_macro(func, l2.begin(), l2.end());
    BOOST_CHECK(structurally_equal(p2.first, l2));
    
    list l3{lit{}};
    auto p3 = execute_macro(func, l3.begin(), l3.end());
    BOOST_CHECK(structurally_equal(p3.first, l3));
}

BOOST_AUTO_TEST_CASE(is_test)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, x, list_get, s, lit{"0"}},
            list{let, a, is_lit, x},
            list{cond_branch, a, block2, block3}
        }},
        list{block2, list
        {
            list{let, b, list_create},
            list{return_symbol, b}
        }},
        list{block3, list
        {
            list{let, c, lit_create},
            list{return_symbol, c}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);

    list l1{lit{}};
    list expected1 = {};
    auto p1 = execute_macro(func, l1.begin(), l1.end());
    BOOST_CHECK(structurally_equal(expected1, p1.first));
    
    list l2{list{}};
    lit expected23 = {};
    auto p2 = execute_macro(func, l2.begin(), l2.end());
    BOOST_CHECK(structurally_equal(expected23, p2.first));

    list l3{ref{""_id}};
    auto p3 = execute_macro(func, l3.begin(), l3.end());
    BOOST_CHECK(structurally_equal(expected23, p3.first));
}

BOOST_AUTO_TEST_CASE(list_push_pop)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, a, list_create},
            list{let, b, lit_create},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_pop, a},
            list{return_symbol, a}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l;
    auto p = execute_macro(func, l.begin(), l.end());
    list expected = {lit{}};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(list_size_test)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, a, list_create},
            list{let, b, lit_create},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{let, c, list_size, a},
            list{let, d, cmp_eq_int64, c, lit{"4"}},
            list{cond_branch, d, block2, block3}
        }},
        list{block2, list
        {
            list{let, x, lit_create},
            list{return_symbol, x}
        }},
        list{block3, list
        {
            list{let, y, list_create},
            list{return_symbol, y}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l;
    auto p = execute_macro(func, l.begin(), l.end());
    lit expected = {};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(list_set_get_test)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, a, list_create},
            list{let, b, lit_create},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{list_push, a, b},
            list{let, c, list_create},
            list{list_set, a, lit{"2"}, c},
            list{let, d, list_get, a, lit{"2"}},
            list{return_symbol, d}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l;
    auto p = execute_macro(func, l.begin(), l.end());
    list expected = {};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(lit_push_pop_test)
{
    const list params =
    {
        list{s, symbol_type}
    };
    const symbol& return_type = symbol_type;
    
    const list body =
    {
        list{block1, list
        {
            list{let, a, lit_create},
            list{lit_push, a, lit{"72"}},
            list{lit_push, a, lit{"101"}},
            list{lit_push, a, lit{"108"}},
            list{lit_push, a, lit{"108"}},
            list{lit_push, a, lit{"111"}},
            list{lit_push, a, lit{"32"}},
            list{lit_push, a, lit{"87"}},
            list{lit_push, a, lit{"111"}},
            list{lit_push, a, lit{"114"}},
            list{lit_push, a, lit{"108"}},
            list{lit_push, a, lit{"100"}},
            list{lit_push, a, lit{"33"}},
            list{lit_push, a, lit{"1"}},
            list{lit_pop, a},
            list{return_symbol, a}
        }}
    };

    const list function_source =
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list l;
    auto p = execute_macro(func, l.begin(), l.end());
    lit expected = {"Hello World!"};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

