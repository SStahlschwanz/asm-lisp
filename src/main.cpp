#include "module.hpp"

#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cerr << "usage: " << argv[0] << " [file]" << endl;
        return 1;
    }
    else
    {
        try
        {
            symbol::list parsed_module = parse_module(argv[1]);
            vector<string> imports = required_modules(parsed_module);
            for(auto s : imports)
                cout << s << endl;
            return 0;
        }
        catch(const parse_error& exc)
        {
            cerr << exc.what() << endl;
            return 1;
        }
        catch(const exception& exc)
        {
            cerr << "error: " << exc.what() << endl;
            return 2;
        }
    }
}
