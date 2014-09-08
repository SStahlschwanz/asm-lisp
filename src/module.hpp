#ifndef MODULE_HPP_
#define MODULE_HPP_

#include "symbol.hpp"
#include "parse_node.hpp"
#include "parse_state.hpp"

#include <fstream>
#include <stdexcept>

class module
{
public:
    module() = delete;
    module(const std::string& file_name)
    {
        std::ifstream fs(file_name);
        if(fs)
        {
            std::istream_iterator<char> begin(fs);
            std::istream_iterator<char> end;
            parse_state<std::istream_iterator<char>> state(begin, end);
            
           module_contents = parse_file(state);
        }
        else
            throw std::runtime_error("file not found: " + file_name);
    }
private:
    std::string file_name;
    symbol module_contents;
};

#endif

