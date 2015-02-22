#include "module.hpp"

#include "error/import_error.hpp"

using std::unordered_map;
using std::string;
using std::vector;

using boost::optional;
using boost::none;

bool is_export_statement(const list_symbol& statement)
{
    return !statement.empty() && statement[0].is_ref() &&
            statement[0].ref().identifier() == "export";
}
bool is_import_statement(const list_symbol& statement)
{
    return !statement.empty() && statement[0].is_ref() &&
            statement[0].ref().identifier() == "import";
}

struct import_statement
{
    const list_symbol& statement;
    const ref_symbol& imported_module;
    const list_symbol& import_list;
};

optional<import_statement> parse_import(const list_symbol& statement)
{
    if(statement.empty() || !statement[0].is_ref() || 
            statement[0].ref().identifier() != "import")
        return none;
    
    if(statement.size() != 4)
        throw import_invalid_argument_number{statement.source()};
    
    const list_symbol& import_list = statement[1].list_else(
            missing_import_list{statement[1].source()});
    for(const symbol& import : import_list)
    {
        if(!import.is_ref())
            throw invalid_imported_identifier{import.source()};
    }
    
    const ref_symbol& from_token = statement[2].ref_else(
            missing_from_token{statement[2].source()});
    if(from_token.identifier() != "from")
        throw missing_from_token{statement[2].source()};

    const ref_symbol& imported_module = statement[3].ref_else(
            invalid_imported_module{statement[3].source()});

    return import_statement{statement, imported_module, import_list};
}

struct export_statement
{
    const list_symbol& statement;
};

optional<export_statement> parse_export(const list_symbol& statement)
{
    if(statement.empty() || !statement[0].is_ref() ||
            statement[0].ref().identifier() != "export")
        return none;

    return export_statement{statement};
}

unordered_map<string, vector<symbol_source>> 
        get_imported_modules(const list_symbol& syntax_tree)
{
    unordered_map<string, vector<symbol_source>> result;
    for(const symbol& s : syntax_tree)
    {
        const list_symbol& statement = s.list();
        if(optional<import_statement> import = parse_import(statement))
        {
            result[import->imported_module.identifier()].push_back(
                    import->statement.source());
        }
        else if(!parse_export(statement))
            break;
    }
    return result;
}



module evaluate_module(list_symbol syntax_tree,
        const unordered_map<string, module>& dependencies)
{
    vector<ref_symbol> exports;
    unordered_map<string, const symbol*> symbol_table;

    bool header_finished = false;
    for(symbol& s : syntax_tree)
    {
        list_symbol& statement = s.list();
        if(optional<import_statement> import = parse_import(statement))
        {
            if(header_finished)
                throw import_after_header{statement.source()};
            assert(dependencies.count(import->imported_module.identifier()));
            for(const symbol& s : import->import_list)
            {
                const ref_symbol& r = s.ref(); // this was checked in parse_import
                
            }
        }
        else if(optional<export_statement> export_statement = parse_export(statement))
        {
            if(header_finished)
                throw export_after_header{statement.source()};
        }
        else
            header_finished = true;

        // TODO
    }
}

