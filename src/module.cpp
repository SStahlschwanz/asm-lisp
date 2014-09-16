#include "module.hpp"

#include "parse_state.hpp"
#include "parse.hpp"
#include "import_error.hpp"

#include <fstream>
#include <stdexcept>

using namespace std;

symbol::list parse_module(const std::string& file_name)
{
    ifstream fs(file_name, ios::binary);
    if(fs)
    {
        istreambuf_iterator<char> begin(fs);
        istreambuf_iterator<char> end;
        parse_state<istreambuf_iterator<char>> state(begin, end);
        return parse_file(state);
    }
    else
        throw runtime_error("file not found: " + file_name);
}

vector<string> required_modules(const symbol::list& parsed_file)
{
    vector<string> result;
    for(const symbol& s : parsed_file)
    {
        const symbol::list* l = boost::get<symbol::list>(&s.content);
        if(l && !l->empty())
        {
            const symbol::reference* import_stmnt = boost::get<symbol::reference>(&l->front().content);
            if(import_stmnt && import_stmnt->identifier == "import")
            {
                for(auto it = l->begin() + 1; it != l->end(); ++it)
                {
                    const symbol& current_symbol = *it;
                    const symbol::reference* module_reference = boost::get<symbol::reference>(&current_symbol.content);
                    if(module_reference)
                        result.push_back(module_reference->identifier);
                    else
                    {
                        ostringstream oss;
                        const source_range& import_location = boost::get<source_range>(l->front().content);
                        const source_range& current_symbol_location = boost::get<source_range>(current_symbol.source);
                        oss << current_symbol_location << ": invalid symbol following import statement at "
                            << import_location << endl;
                        throw import_error(oss.str());
                    }
                }
            }
        }
    }
    
    // remove duplicates
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    
    return result;
}

