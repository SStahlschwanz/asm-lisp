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
            module m(argv[1]);
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
