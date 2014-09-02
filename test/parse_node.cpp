#include <parse_node.hpp>
#include <parse_node.hpp>

#include <parse_state.hpp>

using namespace std;

int main()
{
    typedef parse_state<string::iterator> state;
    {
        string str = "abcd efg";
        state state(str.begin(), str.end());
        symbol result = *parse_node(state);
        assert(boost::get<symbol::reference>(result.content).identifier == "abcd");
        assert(remaining(state) == " efg");
    }
    {
        string str = "\"abcd\" efg";
        state state(str.begin(), str.end());
        symbol result = *parse_node(state);
        assert(boost::get<symbol::literal>(result.content) == "abcd");
        assert(remaining(state) == " efg");
    }
    {
        string str = "{ abc; \"fff\" def; ; } efg";
        state state(str.begin(), str.end());
        symbol result = *parse_node(state);
        assert(remaining(state) == " efg");

        symbol::list content = boost::get<symbol::list>(result.content);
        assert(content.size() == 3);

        symbol::list first_content = boost::get<symbol::list>(content[0].content);
        symbol::list second_content = boost::get<symbol::list>(content[1].content);
        symbol::list third_content = boost::get<symbol::list>(content[2].content);
        assert(first_content.size() == 1);
        assert(second_content.size() == 2);
        assert(third_content.size() == 0);
        assert(boost::get<symbol::reference>(first_content[0].content).identifier == "abc");
        
        assert(boost::get<symbol::literal>(second_content[0].content) == "fff");
        assert(boost::get<symbol::reference>(second_content[1].content).identifier == "def");

    }
    {
        string str = "[ef ak, [a ], [ ] ,]";
        state state(str.begin(), str.end());
        symbol result = *parse_node(state);
        assert(remaining(state) == "");

        symbol::list content = boost::get<symbol::list>(result.content);
        assert(content.size() == 4);

        symbol::list first_content = boost::get<symbol::list>(content[0].content);
        symbol::list second_content = boost::get<symbol::list>(content[1].content);
        symbol::list third_content = boost::get<symbol::list>(content[2].content);
        symbol::list fourth_content = boost::get<symbol::list>(content[3].content);
        assert(first_content.size() == 2);
        assert(second_content.size() == 1);
        assert(third_content.size() == 1);
        assert(fourth_content.size() == 0);
        assert(boost::get<symbol::reference>(first_content[0].content).identifier == "ef");
        assert(boost::get<symbol::reference>(first_content[1].content).identifier == "ak");
        
        assert(boost::get<symbol::list>(second_content[0].content).size() == 1);
        assert(boost::get<symbol::list>(third_content[0].content).size() == 1);
    }

}

