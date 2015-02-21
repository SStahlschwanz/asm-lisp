#define BOOST_TEST_MODULE symbol
#include <boost/test/included/unit_test.hpp>

#include <algorithm>

using std::equal;
using std::begin;

#include "symbol_building.hpp"

BOOST_AUTO_TEST_CASE(none_test)
{
    none_symbol none1;
    none_symbol none2;
    BOOST_CHECK(none1 == none2);
    BOOST_CHECK(!(none1 != none2));
    BOOST_CHECK((symbol&)none1 == (symbol&)none2);
}

BOOST_AUTO_TEST_CASE(lit_test)
{
    lit_symbol lit1 = "abcde";
    const lit_symbol lit2 = lit1;
    lit_symbol lit3 = "asdff";
    
    BOOST_CHECK(std::equal(lit1.begin(), lit1.end(), "abcde"));
    BOOST_CHECK(std::equal(lit2.begin(), lit2.end(), "abcde"));
    BOOST_CHECK(std::equal(lit3.begin(), lit3.end(), "asdff"));

    BOOST_CHECK(lit1[3] == 'd');
    lit3[3] = 'g';
    BOOST_CHECK(std::equal(lit3.begin(), lit3.end(), "asdgf"));

    BOOST_CHECK(lit1 == lit2);
    BOOST_CHECK(lit2 != lit3);
}

BOOST_AUTO_TEST_CASE(ref_test)
{
    ref_symbol ref1{"abcde"};
    const ref_symbol ref2 = ref1;
    ref_symbol ref3{"abcde", &ref1};
    ref_symbol ref4{"asdff", &ref1};
    
    BOOST_CHECK(std::equal(ref1.begin(), ref1.end(), "abcde"));
    BOOST_CHECK(std::equal(ref2.begin(), ref2.end(), "abcde"));
    BOOST_CHECK(std::equal(ref3.begin(), ref3.end(), "abcde"));
    BOOST_CHECK(std::equal(ref4.begin(), ref4.end(), "asdff"));

    BOOST_CHECK(ref1[3] == 'd');
    ref4[3] = 'g';
    BOOST_CHECK(std::equal(ref4.begin(), ref4.end(), "asdgf"));

    BOOST_CHECK(ref1 == ref2);
    BOOST_CHECK(ref2 != ref3);
    BOOST_CHECK(ref3 != ref4);
}

BOOST_AUTO_TEST_CASE(cast_test)
{
    symbol* s;

    lit_symbol lit = "abcde";
    s = &lit;
    BOOST_CHECK_EQUAL(s->type(), symbol::LITERAL);
    BOOST_CHECK_EQUAL(&s->lit(), &lit);

    ref_symbol ref{"abcd"};
    s = &ref;
    BOOST_CHECK_EQUAL(s->type(), symbol::REFERENCE);
    BOOST_CHECK_EQUAL(&s->ref(), &ref);

    list_symbol list;
    s = &list;
    BOOST_CHECK_EQUAL(s->type(), symbol::LIST);
    BOOST_CHECK_EQUAL(&s->list(), &list);
}

BOOST_AUTO_TEST_CASE(visit_test)
{
    bool visited;
    symbol* s;
    
    lit_symbol lit = "abcde";
    s = &lit;
    visited = false;
    s->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &lit);
    });
    BOOST_CHECK(visited);

    ref_symbol ref{"asdf"};
    s = &ref;
    visited = false;
    s->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &ref);
    });
    BOOST_CHECK(visited);
    
    list_symbol list;
    s = &list;
    visited = false;
    s->visit_none([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &list);
    });
    BOOST_CHECK(visited);
}

BOOST_AUTO_TEST_CASE(any_symbol_lit_ref_test) // only test any_symbol with lit_symbol and ref_symbol
{
    any_symbol none_any;
    symbol& none_base = none_any;
    BOOST_CHECK(none_base.type() == symbol::NONE);
    BOOST_CHECK(none_any == none_symbol());
    
    lit_symbol lit_obj = "123";
    any_symbol lit_any = lit_obj;
    BOOST_CHECK(lit_obj == lit_any);
    
    symbol& lit_base = lit_any;
    lit_base.visit_none([&](auto& obj)
    {
        BOOST_CHECK(obj.type() == symbol::LITERAL);
        BOOST_CHECK(obj == lit_obj);
    });

    ref_symbol ref_obj{"123", &lit_obj};
    any_symbol ref_any = ref_obj;
    BOOST_CHECK(ref_obj == ref_any);

    symbol& ref_base = ref_any;
    ref_base.visit_none([&](auto& obj)
    {
        BOOST_CHECK(obj.type() == symbol::REFERENCE);
        BOOST_CHECK(obj == ref_obj);
    });
}

BOOST_AUTO_TEST_CASE(list_symbol_test)
{
    ref_symbol obj1{"abcde"};
    lit_symbol obj2 = "abcde";
    ref_symbol obj3{"abcde", &obj1};
    list_symbol obj4 = {obj2, obj3};
    
    list_symbol list1 = {obj1, obj2, obj4};
    list_symbol list2 = list1;
    list_symbol list3 = {obj1, obj3, obj4};
    
    any_symbol expected1_2[] = {obj1, obj2, obj4};
    BOOST_CHECK(equal(list1.begin(), list1.end(), begin(expected1_2)));
    BOOST_CHECK(equal(list2.begin(), list2.end(), begin(expected1_2)));

    any_symbol expected3[] = {obj1,obj3, obj4};
    BOOST_CHECK(equal(list3.begin(), list3.end(), begin(expected3)));
    list3[2] = obj2;
    any_symbol expected3_now[] = {obj1, obj3, obj2};
    BOOST_CHECK(equal(list3.begin(), list3.end(), begin(expected3_now)));

    BOOST_CHECK(list1 == list2);
    BOOST_CHECK(list2 != list3);
}

