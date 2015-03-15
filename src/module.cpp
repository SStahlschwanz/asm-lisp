#include "module.hpp"

#include "error/import_export_error.hpp"
#include "error/evaluate_error.hpp"

#include <mblib/range.hpp>

#include <string>

using std::unordered_map;
using std::vector;
using std::size_t;
using std::tie;
using std::pair;
using std::ignore;
using std::move;
using std::unique_ptr;
using std::make_unique;
using std::function;
using std::string;

using boost::optional;
using boost::none;

bool is_export_statement(const list_node& statement)
{
    static const string export_str = "export";
    return !statement.empty() && statement[0].is<ref_node>() &&
            statement[0].cast<ref_node>().identifier() == rangeify(export_str);
}
bool is_import_statement(const list_node& statement)
{
    static const string import_str = "import";
    return !statement.empty() && statement[0].is<ref_node>() &&
            statement[0].cast<ref_node>().identifier() == rangeify(import_str);
}

optional<import_statement> parse_import(const list_node& statement)
{
    using namespace import_export_error;
    if(!is_import_statement(statement))
        return none;
    
    if(statement.size() != 4)
        fatal<id("import_invalid_argument_number")>(statement.source(), statement.size() - 1);
    
    const list_node& import_list = statement[1].cast_else<list_node>([&]()
    {
        fatal<id("invalid_import_list")>(statement[1].source());
    });
    
    for(const node& import : import_list)
    {
        if(!import.is<ref_node>())
            fatal<id("invalid_imported_identifier")>(import.source());
    }
    
    const ref_node& from_token = statement[2].cast_else<ref_node>([&]()
    {
        fatal<id("invalid_from_token")>(statement[2].source());
    });
    static const string from_str = "from";
    if(from_token.identifier() != rangeify(from_str))
        fatal<id("invalid_from_token")>(statement[2].source());

    const lit_node& imported_module = statement[3].cast_else<lit_node>([&]()
    {
        fatal<id("invalid_imported_module")>(statement[3].source());
    });

    return import_statement{statement, imported_module, import_list};
}

optional<export_statement> parse_export(const list_node& statement)
{
    if(!is_export_statement(statement))
        return none;

    return export_statement{statement};
}

module_header read_module_header(const list_node& syntax_tree)
{
    module_header header;
    for(const node& s : syntax_tree)
    {
        const list_node& statement = s.cast<list_node>();
        if(optional<import_statement> import = parse_import(statement))
            header.imports.push_back(std::move(*import));
        else if(optional<export_statement> export_st = parse_export(statement))
            header.exports.push_back(std::move(*export_st));
        else
            break;
    }
    return header;
}

unordered_map<string, vector<node_source>> imported_modules(const module_header& header)
{
    unordered_map<string, vector<node_source>> result;
    for(const import_statement& import : header.imports)
        result[save<string>(rangeify(import.imported_module))].push_back(import.statement.source());

    return result;
}

symbol_table initial_symbol_table(const module_header& header, function<const module&(const import_statement&)> get_module_func)
{
    using namespace import_export_error;
    symbol_table table;
    for(const import_statement& import : header.imports)
    {
        const module& imported_module = get_module_func(import);
        for(const node& s : import.import_list)
        {
            // s.cast<ref_node>() checked by parse_import
            auto imported_identifier = save<string>(s.cast<ref_node>().identifier());
            auto symbol_find_it = imported_module.exports.find(imported_identifier);
            if(symbol_find_it == imported_module.exports.end())
                fatal<id("symbol_not_found")>(s.source());
            table.insert({imported_identifier, symbol_find_it->second});
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
                const auto& exported_identifier = exports_it->cast<ref_node>().identifier();
                if(rangeify(identifier) == exported_identifier)
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

void dispatch_references(node& s, const symbol_table& table)
{
    if(s.is<ref_node>())
    {
        ref_node& r = s.cast<ref_node>();
        auto find_it = table.find(save<string>(r.identifier()));
        if(find_it != table.end())
            r.refered(&find_it->second);
    }
    else if(s.is<list_node>())
    {
        list_node& l = s.cast<list_node>();
        for(node& child : l)
            dispatch_references(child, table);
    }
}

module evaluate_module(list_node& syntax_tree, dynamic_graph graph_owner, const module_header& header, function<const module& (const import_statement&)> get_module_func, compilation_context& context)
{
    using namespace evaluate_error;
    symbol_table table = initial_symbol_table(header, get_module_func);

    size_t header_size = header.imports.size() + header.exports.size();
    assert(header_size <= syntax_tree.size());

    dynamic_graph node_owner;

    auto evaluate_macro = [&](auto node_range) -> pair<node&, dynamic_graph>
    {
        dynamic_graph graph;
        auto node_ptr_range = mapped(node_range, [](node& n)
        {
            return &n;
        });
        list_node& l = graph.create_list(save<vector<node*>>(node_ptr_range));
        return {l, move(graph)};
    };

    for(auto it = syntax_tree.begin() + header_size; it != syntax_tree.end(); ++it)
    {
        list_node& statement = it->cast<list_node>();
        if(statement.empty())
            fatal<id("empty_top_level_statement")>(statement.source());
        
        ref_node& command = statement[0].cast_else<ref_node>([&]()
        {
            fatal<id("invalid_command")>(statement[0].source());
        });
        
        static const string def_str = "def";
        if(command.identifier() == rangeify(def_str))
        {
            if(statement.size() < 3)
                fatal<id("def_invalid_argument_number")>(statement.source());

            const ref_node& defined = statement[1].cast_else<ref_node>([&]()
            {
                fatal<id("invalid_defined_symbol")>(statement[1].source());
            });
            for(auto argument_it = statement.begin() + 2; argument_it != statement.end(); ++argument_it)
                dispatch_references(*argument_it, table);
            
            auto p = evaluate_macro(rangeify(statement.begin() + 2, statement.end()));
            node& definition = p.first;
            dynamic_graph& graph = p.second;
            
            graph_owner.add(move(graph));
            
            bool was_inserted;
            tie(ignore, was_inserted) = table.insert({save<string>(defined.identifier()), definition});
            if(!was_inserted)
                fatal<id("duplicate_definition")>(defined.source());
        }
        else if(is_import_statement(statement))
            import_export_error::fatal<import_export_error::id("import_after_header")>(statement.source());
        else if(is_export_statement(statement))
            import_export_error::fatal<import_export_error::id("export_after_header")>(statement.source());
        else
            throw not_implemented{"macro execution without def"};
    }

    remove_not_exported(table, header);
    return module{move(graph_owner), move(table)};
}
