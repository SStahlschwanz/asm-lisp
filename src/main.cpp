#include "compile_unit.hpp"
#include "error/compile_exception.hpp"
#include "printing.hpp"

#include <boost/filesystem.hpp>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/Verifier.h>

#include <iostream>
#include <string>
#include <fstream>

using boost::filesystem::path;

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::pair;
using std::ofstream;
using std::ios;
using std::size_t;

using llvm::raw_os_ostream;
using llvm::WriteBitcodeToFile;
using llvm::verifyModule;

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
        for_each(zipped(paths, modules), unpacking(
        [&](const path& p, const module& m)
        {
            cout << "file " << p.native() << ":" << endl;
            for_each(m.exports, unpacking(
            [&](const string& identifier, const node& n)
            {
                cout << identifier << " defined as\n";
                cout << n;
            }));
            cout << endl;
        }));
    }
    catch(const compile_exception& exc)
    {
        auto file_id_to_name = [&](size_t file_id) -> string
        {
            assert(file_id < paths.size());
            return paths[file_id].native();
        };
        print(cerr, exc, file_id_to_name);
    }

    raw_os_ostream llvm_cerr{cerr};
    assert(!verifyModule(context.runtime_module(), &llvm_cerr)); // yes, this returns false when module is actually correct

    ofstream output{"output.bc", ios::binary};
    raw_os_ostream llvm_output{output};

    WriteBitcodeToFile(&context.runtime_module(), llvm_output);
}

