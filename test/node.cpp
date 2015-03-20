#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE parse
#include <boost/test/unit_test.hpp>

#include "graph_building.hpp"
#include "../src/node.hpp"

using std::string;
using std::exception;

using boost::get;

BOOST_AUTO_TEST_CASE(asdf_test)
{
    node& n1 = list{};
    BOOST_CHECK(n1.is<list_node>());

    string str = "123";
    node& n2 = lit{str};
    BOOST_CHECK(n2.is<lit_node>());
    BOOST_CHECK(rangeify(n2.cast<lit_node>()) == rangeify(str));

    node& n3 = ref{str};
    BOOST_CHECK(n3.is<ref_node>());
    BOOST_CHECK(n3.cast<ref_node>().identifier() == rangeify(str));

    node& n4 = id{5};
    BOOST_CHECK(n4.is<id_node>());
    BOOST_CHECK(n4.cast<id_node>().id() == 5);
}

BOOST_AUTO_TEST_CASE(cast_test)
{
    struct my_exception
      : exception
    {};
    auto thrower = []()
    {
        throw my_exception{};
    };
    node* n;

    lit_node& lit_obj = lit{"abcde"};
    n = &lit_obj;
    BOOST_CHECK(n->type() == node_type::LITERAL);
    BOOST_CHECK_EQUAL(&n->cast<lit_node>(), &lit_obj);
    BOOST_CHECK_EQUAL(&n->cast_else<lit_node>(thrower), &lit_obj);
    BOOST_CHECK_THROW(n->cast_else<ref_node>(thrower), my_exception);

    ref_node& ref_obj = ref{"adss"};
    n = &ref_obj;
    BOOST_CHECK(n->type() == node_type::REFERENCE);
    BOOST_CHECK_EQUAL(&n->cast<ref_node>(), &ref_obj);

    list_node& list_obj = list{};
    n = &list_obj;
    BOOST_CHECK(n->type() == node_type::LIST);
    BOOST_CHECK_EQUAL(&n->cast<list_node>(), &list_obj);
}

BOOST_AUTO_TEST_CASE(visit_test)
{
    bool visited;
    node* n;
    
    lit_node& lit_obj = lit{"abcde"};
    n = &lit_obj;
    visited = false;
    n->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &lit_obj);
    });
    BOOST_CHECK(visited);

    ref_node& ref_obj = ref{"22"};
    n = &ref_obj;
    visited = false;
    n->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &ref_obj);
    });
    BOOST_CHECK(visited);
    
    list_node& list_obj = list{};
    n = &list_obj;
    visited = false;
    n->visit([&](auto& obj)
    {
        visited = true;
        BOOST_CHECK((void*)&obj == &list_obj);
    });
    BOOST_CHECK(visited);
}

BOOST_AUTO_TEST_CASE(list_symbol_equality_test)
{
    ref_node& obj1 = ref{"234234"};
    lit_node& obj2 = lit{"abcde"};
    ref_node& obj3 = ref{"343423", &obj1};
    list_node& obj4 = list{obj2, obj3};
    
    list_node& list1 = list{obj1, obj2, obj4};
    list_node& list2 = list{list1};
    list_node& list3 = list{obj1, obj3, obj4};
    
    BOOST_CHECK(!structurally_equal(list1, list3));
    BOOST_CHECK(!structurally_equal(list1, list2));
}

#include <iostream>
#include "../src/printing.hpp"
using std::cout;
using std::endl;

BOOST_AUTO_TEST_CASE(dynamic_graph_cloning_test)
{
    dynamic_graph graph;
    lit_node& lit1 = graph.create_lit("lit1");
    ref_node& ref1 = graph.create_ref("ref1");
    ref1.refered(&lit1);
    list_node& list1 = graph.create_list({&lit1, &ref1});

    dynamic_graph clone{list1};
    BOOST_CHECK(clone.data.size() == 3);
    list_node& cloned_list1 = get<dynamic_graph::list_data>(*clone.data.front()).first;
    BOOST_CHECK(length(rangeify(cloned_list1)) == 2);
    cout << cloned_list1;
    BOOST_CHECK(structurally_equal(cloned_list1, list1));
}
