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
