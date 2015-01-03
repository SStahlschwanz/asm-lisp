#include "module.hpp"

#include "parse_state.hpp"
#include "parse.hpp"
#include "import_error.hpp"

#include <fstream>
#include <stdexcept>

using namespace std;

boost::optional<std::vector<std::string>> parse_import_statement(const symbol::list& statement)
{
    if(statement.empty())
        return boost::none;

    const symbol::reference* import_symbol = statement[0].cast_reference();
    
    if(import_symbol == nullptr || import_symbol->identifier != "import")
        return boost::none;
    
    if(statement.size() == 1)
    {
        std::ostringstream error_builder;
        error_builder << boost::get<source_range>(statement[0].source) << ": empty import statement";
        throw import_error(error_builder.str());
    }

    std::vector<std::string> result;
    for(auto it = statement.begin() + 1; it != statement.end(); ++it)
    {
        const symbol::reference* module_name = it->cast_reference();
        if(!module_name)
        {
            std::ostringstream error_builder;
            error_builder << boost::get<source_range>(it->source) << ": invalid argument for 'import' statement";
            throw import_error(error_builder.str());
        }
        result.push_back(module_name->identifier);
    }
    return result;
}

vector<string> required_modules(const symbol::list& parsed_file)
{
    vector<string> result;
    for(const symbol& s : parsed_file)
    {
        const symbol::list* l = s.cast_list();

        assert(l != nullptr);
        boost::optional<vector<string>> import_parse_result = parse_import_statement(*l);
        if(import_parse_result)
            result.insert(result.end(), import_parse_result->begin(), import_parse_result->end());
    }
    
    // remove duplicates
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    
    return result;
}

module read_module(const std::string& file_name)
{
    ifstream fs(file_name, ios::binary);
    if(!fs)
        throw runtime_error("unable to open file: " + file_name);
    
    istreambuf_iterator<char> begin(fs);
    istreambuf_iterator<char> end;
    
    module result;
    
    parse_state<istreambuf_iterator<char>> state(begin, end);
    result.syntax_tree = parse_file(state);
    
    result.required_modules = required_modules(result.syntax_tree);

    return result;
}
