#include "compile_unit.hpp"
#include "error/compile_exception.hpp"
#include "error_to_string.hpp"

#include <boost/filesystem.hpp>

#include <iostream>
#include <string>

using boost::filesystem::path;

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::pair;

int main(int argc, char** args)
{
    vector<path> paths{args + 1, args + argc};
    cout << "compiling files";
    for(const path& p : paths)
        cout << " " << p.native();
    cout << endl;
    
    compilation_context context;
    try
    {
        vector<module> modules = compile_unit(paths, context);
        assert(modules.size() == paths.size());
        for(size_t i = 0; i != modules.size(); ++i)
        {
            cout << "file " << paths[i].native() << ":" << endl;
            module& m = modules[i];
            for(const pair<identifier_id_t, const symbol*>& exp : m.exports)
            {
                const string& identifier = context.to_string(exp.first);
                const symbol& s = *exp.second;
                cout << identifier << " defined as" << endl;
                print_symbol(cout, s, context);
                cout << endl;
            }
            cout << endl;
        }
    }
    catch(const compile_exception& exc)
    {
        auto file_id_to_name = [&](size_t file_id) -> string
        {
            assert(file_id < paths.size());
            return paths[file_id].native();
        };
        cerr << default_error_to_string(exc, file_id_to_name) << endl; 
    }
}
