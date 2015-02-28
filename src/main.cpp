#include "parse_state.hpp"
#include "parse.hpp"
#include "error/compile_exception.hpp"
#include "error_to_string.hpp"
#include "module.hpp"

#include <iostream>
#include <iterator>
#include <fstream>

using std::fstream;
using std::cerr;
using std::endl;

using namespace std;

int main(int argc, char** args)
{
    char** end = args + argc;
    unordered_map<size_t, string> file_id_name_mapping;
    compilation_context context;
    unordered_map<size_t, module> module_map;
    for(char** it = args + 1; it != end; ++it)
    {
        size_t file_id = context.identifier_id(*it);
        file_id_name_mapping[file_id] = *it;
        try
        {
            fstream file(*it);
            istream_iterator<char> begin{file};
            istream_iterator<char> end{};
            parse_state<istream_iterator<char>> state{begin, end, file_id, context};
            list_symbol syntax_tree = parse_file(state);
            module_header header = read_module_header(syntax_tree);
            module m = evaluate_module(move(syntax_tree), header, module_map, context);
            module_map.emplace(file_id, move(m));
        }
        catch(const compile_exception& exc)
        {
            cout << default_error_to_string(exc, file_id_name_mapping) << endl;
        }
    }
}
