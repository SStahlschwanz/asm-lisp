#include "module.hpp"

#include "error/import_export_exception.hpp"
#include "error/evaluation_exception.hpp"

using std::unordered_map;
using std::string;
using std::vector;
using std::size_t;

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

optional<import_statement> parse_import(const list_symbol& statement)
{
    if(!is_import_statement(statement))
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

optional<export_statement> parse_export(const list_symbol& statement)
{
    if(!is_export_statement(statement))
        return none;

    return export_statement{statement};
}

module_header read_module_header(const list_symbol& syntax_tree)
{
    module_header header;
    for(const symbol& s : syntax_tree)
    {
        const list_symbol& statement = s.list();
        if(optional<import_statement> import = parse_import(statement))
            header.imports.push_back(std::move(*import));
        else if(optional<export_statement> export_st = parse_export(statement))
            header.exports.push_back(std::move(*export_st));
        else
            break;
    }
    return header;
}

unordered_map<string, vector<symbol_source>> imported_modules(const module_header& header)
{
    unordered_map<string, vector<symbol_source>> result;
    for(const import_statement& import : header.imports)
    {
        result[import.imported_module.identifier()].push_back(
                import.statement.source());
    }
    return result;
}

unordered_map<string, const symbol*> initial_symbol_table(const module_header& header,
        const unordered_map<string, module>& dependencies)
{
    unordered_map<string, const symbol*> symbol_table;
    for(const import_statement& import : header.imports)
    {
        const auto& module_name = import.imported_module.identifier();
        auto module_find_it = dependencies.find(module_name);
        assert(module_find_it != dependencies.end()); // this has to be checked by the caller
        const module& imported_module = module_find_it->second;
        for(const symbol& s : import.import_list)
        {
            // s.ref() checked by parse_import
            const auto& imported_identifier = s.ref().identifier();
            auto symbol_find_it = imported_module.exports.find(imported_identifier);
            if(symbol_find_it == imported_module.exports.end())
                throw symbol_not_found{s.source()};
            symbol_table[imported_identifier] = symbol_find_it->second;
        }
    }
    return symbol_table;
}

void remove_not_exported(symbol_table& table, const module_header& header)
{
    for(auto it = table.begin(); it != table.end(); )
    {
        const auto& identifier = it->first;
        for(const export_statement& export_st : header.exports)
        {
            for(auto exports_it = export_st.statement.begin() + 1;
                    exports_it != export_st.statement.end();
                    ++it)
            {
                const auto& exported_identifier = exports_it->ref().identifier();
                if(identifier == exported_identifier)
                {
                    ++it;
                    goto is_exported;
                }
            }
        }
        it = table.erase(it);
        is_exported:;
    }
}

module evaluate_module(list_symbol syntax_tree, const module_header& header,
        const unordered_map<string, module>& dependencies)
{
    symbol_table table = initial_symbol_table(header, dependencies);

    size_t header_size = header.imports.size() + header.exports.size();
    for(auto it = syntax_tree.begin() + header_size; it != syntax_tree.end(); ++it)
    {
        const list_symbol& statement = it->list();
        if(statement.empty())
            throw empty_top_level_statement{statement.source()};

    }

    remove_not_exported(table, header);
    return module{std::move(syntax_tree), std::move(table), {}};
}
