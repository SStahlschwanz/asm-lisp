# include "module.hpp"
#include "parse_state.hpp"

#include <iostream>
#include <iterator>


using namespace std;

int main()
{
    istream_iterator<char> begin{std::cin};
    istream_iterator<char> end{};
    
    parse_state<istream_iterator<char>> state{begin, end, 1};
    
    try
    {
        parse_file(state);
    } catch(const parse_exception& exc)
    {
        cerr << "parse error" << endl;
    }
}
