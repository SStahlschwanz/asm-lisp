#include "module.hpp"

module parse_module(const std::string& file_name)
{
    std::ifstream fs(file_name, std::ios::binary);
    if(fs)
    {
        module result;
        std::istreambuf_iterator<char> begin(fs);
        std::istreambuf_iterator<char> end;
        parse_state<std::istreambuf_iterator<char>> state(begin, end);
        return module{file_name, {}, parse_file(state)};
    }
    else
        throw std::runtime_error("file not found: " + file_name);
}
