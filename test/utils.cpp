#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE utils
#include <boost/test/unit_test.hpp>

#include "../src/utils/boost_variant_utils.hpp"
#include "../src/utils/tuple_utils.hpp"
#include "../src/range.hpp"

#include <vector>
#include <string>

using std::tuple;
using std::get;
using std::vector;
using std::string;
using std::to_string;
using std::size_t;
using std::stoul;
using std::make_tuple;
using std::is_same;
using std::decay_t;

BOOST_AUTO_TEST_CASE(apply_test)
{
    auto f = [](const string& str, int i, size_t f)
    {
        return str + to_string(i) + to_string(f);
    };

    tuple<string, int, size_t> t{"abc", -3, 5};
    BOOST_CHECK_EQUAL(tuple_apply(f, t), "abc-35");
}

struct map_functor
{
    unsigned long operator()(const string& str)
    {
        return stoul(str);
    }
    bool operator()(size_t number)
    {
        return number == 42;
    }
    int& operator()(int& number)
    {
        return number;
    }
};

BOOST_AUTO_TEST_CASE(map_test)
{
    tuple<size_t, string, int> t{42, "123", -3};   
    auto result = tuple_map(t, map_functor{});
    
    typedef tuple<bool, unsigned long, int&> expected_type;
    BOOST_CHECK((is_same<decltype(result), expected_type>{}));

    BOOST_CHECK(result == expected_type(true, 123, get<2>(t)));
}


BOOST_AUTO_TEST_CASE(foldl_test)
{
    auto functor = overloaded<bool>(
    [](bool last_result, const string& obj) -> bool
    {
        return last_result && obj == "1";
    },
    [](bool last_result, size_t obj) -> bool
    {
        return last_result && obj == 2;
    },
    [](bool last_result, int obj) -> bool
    {
        return last_result && obj == 3;
    });
    
    typedef tuple<size_t, string, int> tuple_type;
    tuple_type  t1{42, "123", -3};   
    BOOST_CHECK(!tuple_foldl(true, t1, functor));

    tuple_type  t2{2, "1", 3};   
    BOOST_CHECK(tuple_foldl(true, t2, functor));
}

BOOST_AUTO_TEST_CASE(zipped_test)
{
    vector<int> v1{-1, -2, -3};
    vector<string> v2{"1", "2", "3"};
    vector<size_t> v3{1, 2, 3};

    auto z = zipped(rangeify(v1), rangeify(v2), rangeify(v3));
    for_each(z, unpacking([&](int& i, string& str, size_t& s)
    {
        BOOST_CHECK_EQUAL((size_t)-i, s); 
        BOOST_CHECK_EQUAL(to_string(s), str);

        str = to_string(i);
        i = -i;
    }));
    for_each(z, unpacking([&](int& i, string& str, size_t& s)
    {
        BOOST_CHECK_EQUAL(to_string(-i), str);
    }));
}

#include <iostream>
BOOST_AUTO_TEST_CASE(filtered_test)
{
    vector<size_t> v;
    for(size_t i = 0; i != 10; ++i)
        v.push_back(i);
    auto odds = filtered(v, [](size_t i)
    {
        return i % 2 != 0;
    });
    vector<size_t> expected = {1, 3, 5, 7, 9};

    BOOST_CHECK(odds == rangeify(expected));
}

BOOST_AUTO_TEST_CASE(range_traits_test)
{
    auto test = [](auto&& obj)
    {
        as_range<decay_t<decltype(obj)>> r(obj);
        assert(r.empty());
        if(!r.empty())
        {
            r.pop_front();
            r.front();
        }
    };

    struct a_range
    {
        typedef range_tag range_tag;

        void pop_front()
        {}
        bool empty() const
        {
            return true;
        }
        bool front() const
        {
            return true;
        }
    };
    typedef std::vector<int> not_a_range;

    BOOST_CHECK(is_range_t<a_range>{});
    BOOST_CHECK(!is_range_t<not_a_range>{});
    
    BOOST_CHECK((is_same<as_range<a_range>, a_range>{}));
    BOOST_CHECK((is_same<as_range<not_a_range>, iterator_range<not_a_range::iterator>>{}));

    test(a_range{});
    test(not_a_range{});
}

