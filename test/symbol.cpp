#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <exception>

#include "../src/symbol.hpp"
 
using std::equal;
using std::begin;
using std::exception;

typedef lit_symbol lit;
typedef ref_symbol ref;
typedef list_symbol list;

BOOST_AUTO_TEST_CASE(lit_test)
{
    lit lit1 = "abcde";
    const lit lit2 = lit1;
    lit lit3 = "asdff";
    
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
    ref ref1{1};
    const ref ref2 = ref1;
    ref ref3{1, &ref1};
    ref ref4{2, &ref1};
    
    BOOST_CHECK_EQUAL(ref1.identifier(), 1);
    BOOST_CHECK_EQUAL(ref2.identifier(), 1);
    BOOST_CHECK_EQUAL(ref3.identifier(), 1);
    BOOST_CHECK_EQUAL(ref4.identifier(), 2);
    
    BOOST_CHECK_EQUAL(ref3.refered(), &ref1);
    BOOST_CHECK_EQUAL(ref4.refered(), &ref1);

    BOOST_CHECK(ref1 == ref2);
    BOOST_CHECK(ref2 != ref3);
    BOOST_CHECK(ref3 != ref4);
}

BOOST_AUTO_TEST_CASE(cast_test)
{
    struct my_exception
      : exception
    {};
    my_exception exc;
    
    symbol* s;

    lit lit_obj = "abcde";
    s = &lit_obj;
    BOOST_CHECK_EQUAL(s->type(), symbol::LITERAL);
    BOOST_CHECK_EQUAL(&s->cast<lit>(), &lit_obj);
    BOOST_CHECK_EQUAL(&s->cast_else<lit>(exc), &lit_obj);
    BOOST_CHECK_THROW(s->cast_else<ref>(exc), my_exception);

    ref ref_obj{1};
    s = &ref_obj;
    BOOST_CHECK_EQUAL(s->type(), symbol::REFERENCE);
    BOOST_CHECK_EQUAL(&s->cast<ref>(), &ref_obj);

    list list_obj;
    s = &list_obj;
    BOOST_CHECK_EQUAL(s->type(), symbol::LIST);
    BOOST_CHECK_EQUAL(&s->cast<list>(), &list_obj);
}

BOOST_AUTO_TEST_CASE(visit_test)
{
    bool visited;
    symbol* s;
    
    lit lit_obj = "abcde";
    s = &lit_obj;
    visited = false;
    s->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &lit_obj);
    });
    BOOST_CHECK(visited);

    ref ref_obj{1};
    s = &ref_obj;
    visited = false;
    s->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &ref_obj);
    });
    BOOST_CHECK(visited);
    
    list list_obj;
    s = &list_obj;
    visited = false;
    s->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &list_obj);
    });
    BOOST_CHECK(visited);
}

BOOST_AUTO_TEST_CASE(any_symbol_lit_ref_test) // only test any_symbol with lit_symbol and ref_symbol
{
    lit lit_obj = "123";
    any_symbol lit_any = lit_obj;
    BOOST_CHECK(lit_obj == lit_any);
    
    symbol& lit_base = lit_any;
    lit_base.visit([&](auto& obj)
    {
        BOOST_CHECK(obj.type() == symbol::LITERAL);
        BOOST_CHECK(obj == lit_obj);
    });

    ref ref_obj{1, &lit_obj};
    any_symbol ref_any = ref_obj;
    BOOST_CHECK(ref_obj == ref_any);

    symbol& ref_base = ref_any;
    ref_base.visit([&](auto& obj)
    {
        BOOST_CHECK(obj.type() == symbol::REFERENCE);
        BOOST_CHECK(obj == ref_obj);
    });
}

BOOST_AUTO_TEST_CASE(list_symbol_test)
{
    ref obj1{1};
    lit obj2 = "abcde";
    ref obj3{1, &obj1};
    list obj4 = {obj2, obj3};
    
    list list1 = {obj1, obj2, obj4};
    list list2 = list1;
    list list3 = {obj1, obj3, obj4};
    
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
