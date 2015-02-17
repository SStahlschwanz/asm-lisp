# include "module.hpp"
#include "parse_state.hpp"

#include <iostream>
#include <iterator>


using namespace std;

int main()
{
    istream_iterator<char> begin{std::cin};
    istream_iterator<char> end{};
    
    parse_state<istream_iterator<char>> state{begin, end, "command line"};
    
    try
    {
        parse_file(state);
    } catch(const parse_exception& exc)
    {
        cerr << "command line:" << exc.pos.line << ":" << exc.pos.line_position 
                << ": error: " << parse_error_strings[(unsigned int)exc.error] << endl;
    }
}
