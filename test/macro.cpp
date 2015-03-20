#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE macro
#include <boost/test/unit_test.hpp>

#include "../src/macro_execution.hpp"

#include "graph_building.hpp"
#include "function_building.hpp"

BOOST_AUTO_TEST_CASE(parameter_return_test)
{
    list_node& params = list
    {
        list{s, node_type}
    };
    node& return_type = node_type;
    list_node& body = list
    {
        list{block1, list
        {
            list{return_symbol, s}
        }}
    };
    
    list_node& function_source = list
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    list_node& l1 = list{};
    auto p1 = execute_macro(func, rangeify(l1));
    BOOST_CHECK(structurally_equal(p1.first, l1));
    
    const list_node& l2 = list{lit{"abc"}, ref{"123", &l1}};
    auto p2 = execute_macro(func, rangeify(l2));
    BOOST_CHECK(structurally_equal(p2.first, l2));
    
    const list_node& l3 = list{lit{""}};
    auto p3 = execute_macro(func, rangeify(l3));
    BOOST_CHECK(structurally_equal(p3.first, l3));
}

BOOST_AUTO_TEST_CASE(is_test)
{
    list_node& params = list
    {
        list{s, node_type}
    };
    node& return_type = node_type;
    
    list_node& body = list
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

    list_node& function_source = list
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);

    const list_node& l1 = list{lit{""}};
    list_node& expected1 = list{};
    auto p1 = execute_macro(func, rangeify(l1));
    BOOST_CHECK(structurally_equal(expected1, p1.first));
    
    const list_node& l2 = list{list{}};
    lit_node& expected23 = lit{""};
    auto p2 = execute_macro(func, rangeify(l2));
    BOOST_CHECK(structurally_equal(expected23, p2.first));

    const list_node& l3 = list{ref{""}};
    auto p3 = execute_macro(func, rangeify(l3));
    BOOST_CHECK(structurally_equal(expected23, p3.first));
}

BOOST_AUTO_TEST_CASE(list_push_pop)
{
    list_node& params = list
    {
        list{s, node_type}
    };
    node& return_type = node_type;
    
    list_node& body = list
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

    list_node& function_source = list
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    const list_node& l = list{};
    auto p = execute_macro(func, rangeify(l));
    list_node& expected = list{lit{""}};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(list_size_test)
{
    list_node& params = list
    {
        list{s, node_type}
    };
    node& return_type = node_type;
    
    list_node& body = list
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

    list_node& function_source = list
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    const list_node& l = list{};
    auto p = execute_macro(func, rangeify(l));
    lit_node& expected = lit{""};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(list_set_get_test)
{
    list_node& params = list
    {
        list{s, node_type}
    };
    node& return_type = node_type;
    
    list_node& body = list
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

    list_node& function_source = list
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    const list_node& l = list{};
    auto p = execute_macro(func, rangeify(l));
    list expected = {};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

BOOST_AUTO_TEST_CASE(lit_push_pop_test)
{
    list_node& params = list
    {
        list{s, node_type}
    };
    node& return_type = node_type;
    
    list_node& body = list
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

    list_node& function_source = list
    {
        params,
        return_type,
        body
    };

    macro_function* func = get_compiled_function<macro_function>(function_source);
    const list_node& l = list{};
    auto p = execute_macro(func, rangeify(l));
    lit expected = {"Hello World!"};
    BOOST_CHECK(structurally_equal(p.first, expected));
}

