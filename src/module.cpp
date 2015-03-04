#include "module.hpp"

#include "error/import_export_error.hpp"
#include "error/evaluate_error.hpp"

using std::unordered_map;
using std::vector;
using std::size_t;
using std::tie;
using std::ignore;
using std::move;
using std::unique_ptr;
using std::make_unique;
using std::function;

using boost::optional;
using boost::none;

using namespace symbol_shortcuts;

bool is_export_statement(const list_symbol& statement)
{
    return !statement.empty() && statement[0].is<ref>() &&
            statement[0].cast<ref>().identifier() == static_cast<size_t>(identifier_ids::EXPORT);
}
bool is_import_statement(const list_symbol& statement)
{
    return !statement.empty() && statement[0].is<ref>() &&
            statement[0].cast<ref>().identifier() == static_cast<size_t>(identifier_ids::IMPORT);
}

optional<import_statement> parse_import(const list_symbol& statement)
{
    using namespace import_export_error;
    if(!is_import_statement(statement))
        return none;
    
    if(statement.size() != 4)
        fatal<id("import_invalid_argument_number")>(statement.source());
    
    const list_symbol& import_list = statement[1].cast_else<list>([&]()
    {
        fatal<id("invalid_import_list")>(statement[1].source());
    });
    
    for(const symbol& import : import_list)
    {
        if(!import.is<ref>())
            fatal<id("invalid_imported_identifier")>(import.source());
    }
    
    const ref_symbol& from_token = statement[2].cast_else<ref>([&]()
    {
        fatal<id("invalid_from_token")>(statement[2].source());
    });
    if(from_token.identifier() != static_cast<size_t>(identifier_ids::FROM))
        fatal<id("invalid_from_token")>(statement[2].source());

    const ref_symbol& imported_module = statement[3].cast_else<ref>([&]()
    {
        fatal<id("invalid_imported_module")>(statement[3].source());
    });

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
        const list_symbol& statement = s.cast<list>();
        if(optional<import_statement> import = parse_import(statement))
            header.imports.push_back(std::move(*import));
        else if(optional<export_statement> export_st = parse_export(statement))
            header.exports.push_back(std::move(*export_st));
        else
            break;
    }
    return header;
}

unordered_map<identifier_id_t, vector<symbol_source>> imported_modules(const module_header& header)
{
    unordered_map<identifier_id_t, vector<symbol_source>> result;
    for(const import_statement& import : header.imports)
    {
        result[import.imported_module.identifier()].push_back(
                import.statement.source());
    }
    return result;
}

symbol_table initial_symbol_table(const module_header& header, function<const module&(const import_statement&)> get_module_func)
{
    using namespace import_export_error;
    symbol_table table;
    for(const import_statement& import : header.imports)
    {
        const module& imported_module = get_module_func(import);
        for(const symbol& s : import.import_list)
        {
            // s.cast<ref>() checked by parse_import
            const auto& imported_identifier = s.cast<ref>().identifier();
            auto symbol_find_it = imported_module.exports.find(imported_identifier);
            if(symbol_find_it == imported_module.exports.end())
                fatal<id("symbol_not_found")>(s.source());
            table[imported_identifier] = symbol_find_it->second;
        }
    }
    return table;
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
                    ++exports_it)
            {
                const auto& exported_identifier = exports_it->cast<ref>().identifier();
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

void dispatch_references(symbol& s, const symbol_table& table,
        vector<unique_ptr<any_symbol>>& evaluated_symbols, compilation_context& context)
{
    if(s.is<ref>())
    {
        ref_symbol& r = s.cast<ref>();
        auto find_it = table.find(r.identifier());
        if(find_it != table.end())
            r.refered(find_it->second);
    }
    else if(s.is<list>())
    {
        list_symbol& l = s.cast<list>();
        for(symbol& child : l)
            dispatch_references(child, table, evaluated_symbols, context);
    }
}

module evaluate_module(list_symbol syntax_tree, const module_header& header, function<const module&(const import_statement&)> get_module_func, compilation_context& context)
{
    using namespace evaluation_error;
    symbol_table table = initial_symbol_table(header, get_module_func);

    size_t header_size = header.imports.size() + header.exports.size();
    assert(header_size <= syntax_tree.size());

    vector<unique_ptr<any_symbol>> evaluated_symbols;
    for(auto it = syntax_tree.begin() + header_size; it != syntax_tree.end(); ++it)
    {
        list_symbol& statement = it->cast<list>();
        if(statement.empty())
            fatal<id("empty_top_level_statement")>(statement.source());
        
        ref_symbol& command = statement[0].cast_else<ref>([&]()
        {
            fatal<id("invalid_command")>(statement[0].source());
        });

        if(command.identifier() == static_cast<size_t>(identifier_ids::DEF))
        {
            if(statement.size() < 3)
                fatal<id("def_invalid_argument_number")>(statement.source());

            const ref_symbol& defined = statement[1].cast_else<ref>([&]()
            {
                fatal<id("invalid_defined_symbol")>(statement[1].source());
            });
            for(auto argument_it = statement.begin() + 2;
                    argument_it != statement.end();
                    ++argument_it)
            {
                dispatch_references(*argument_it, table, evaluated_symbols, context);
            }
            
            const symbol* definition = 0;
            if(statement.size() == 3)
                definition = &static_cast<const symbol&>(statement[2]);
            else
                assert(false);
            
            bool was_inserted;
            tie(ignore, was_inserted) = table.insert({defined.identifier(), definition});
            if(!was_inserted)
                fatal<id("duplicate_definition")>(defined.source());
        }
        else if(command.identifier() == static_cast<size_t>(identifier_ids::IMPORT))
            import_export_error::fatal<import_export_error::id("import_after_header")>(statement.source());
        else if(command.identifier() == static_cast<size_t>(identifier_ids::EXPORT))
            import_export_error::fatal<import_export_error::id("export_after_header")>(statement.source());
    }

    remove_not_exported(table, header);
    return module{move(syntax_tree), move(table), move(evaluated_symbols)};
}
