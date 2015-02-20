#include "module.hpp"

#include "parse_state.hpp"
#include "parse.hpp"
#include "error/import_error.hpp"
#include "error/eval_error.hpp"
#include "define_error.hpp"

#include <boost/optional.hpp>

#include <utility>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstring>

using boost::optional;
using boost:none;

using std::vector;
using std::string;
using std::strcmp;
using std::istream_iterator;
using std::unordered_map;
using std::pair;
using std::sort;
using std::unique;
using std::strcmp;

void sort_and_unique(vector<module::import_entry>& imports)
{
    // TODO: inefficient
    sort(imports.begin(), imports.end(), [](auto& lhs, auto& rhs)
    {
        return strcmp(lhs.module, rhs.module) < 0 || strcmp(lhs.identifier, rhs.identifier) < 0;
    });

    imports.erase(unique(imports.begin(), imports.end(), [](auto& lhs, auto& rhs)
    {
        return strcmp(lhs.module, rhs.module) == 0 && strcmp(lhs.identifier, rhs.identifier) == 0;
    }), imports.end());
}

bool is_export_statement(const symbol::list& statement)
{
    if(statement.empty())
        return false;
    if(const symbol::reference* identifier = statement.front().cast_reference())
    {
        if(identifier->identifier == "export")
            return true;
    }

    return false;
}

optional<vector<const symbol::reference*>> parse_export_statement(const symbol& statement)
{
    const symbol::list* l = statement.cast_list();
    if(l == nullptr || l->empty())
        return none;
    
    const symbol::reference* export_identifier = (*l)[0].cast_reference();
    if(export_identifier == nullptr)
        return none;

    vector<const symbol::reference*> result;
    for(auto it = l->begin() + 1; it != l->end(); ++it)
    {
        const symbol::reference* exported = it->cast_reference();
        if(exported == nullptr)
            throw import_exception(it->source, import_error::INVALID_EXPORT_IDENTIFIER);
        result.push_back(exported);
    }
    return result;
}

optional<pair<const symbol::reference*, vector<const symbol::reference*>>> parse_import_statement(const symbol& statement)
{
    const symbol::list* l = statement.cast_list();
    if(l == nullptr || l->empty())
        return none;
    
    const symbol::reference* import_identifier = (*l)[0].cast_reference();
    if(import_identifier == nullptr)
        return none;
    
    if((*l).size() != 4)
        throw import_exception{(*l)[0].source , import_error::INVALID_ARGUMENT_NUMBER};

    const symbol::list* import_list = (*l)[1].cast_list();
    if(import_list == nullptr)
        throw import_exception{(*l)[1].source, import_error::INVALID_IMPORT_LIST};

    const symbol::reference* from_identifier = (*l)[2].cast_reference();
    if(from_identifier == nullptr)
        throw import_exception{(*l)[2].source, import_error::MISSING_FROM_IDENTIFIER};

    const symbol::reference* module_name = (*l)[3].cast_reference();
    if(module_name == nullptr)
        throw import_exception{(*l)[3].source, import_error::INVALID_MODULE_NAME};
    
    pair<const symbol::reference*, vector<const symbol::reference*>> result;
    result.first = module_name;
    for(const symbol& s : *import_list)
    {
        const symbol::reference* import = s.cast_reference();
        if(import == nullptr)
            throw import_exception{s.source, import_error::INVALID_IMPORT_IDENTIFIER};
        result.second.push_back(import);
    }

    return result;
}

unordered_map<string, symbol::source_type> module::get_imported_modules() const
{
    unordered_map<string, const module*> imported_modules;
    for(const symbol& statement : syntax_tree)
    {
    }

    return imported_modules;
}

void dispatch_references(symbol& s, unordered_map<string, const symbol*>& symbol_table)
{
    if(symbol::list* l = s.cast_list())
    {
        for(symbol& child : *l)
            dispatch_references(child, symbol_table);
    }
    else if(symbol::reference* l = s.cast_reference())
    {
        auto symbol_it = symbol_table.find(l->identifier);
        if(symbol_it != symbol_table.end())
            l->refered = symbol_it->second;
    }
}

void module::evaluate_exports(const unordered_map<string, const module::export_table*>& imported_tables)
{
    // initialize temporary symbol table for symbols in this module
    // (not necessarily exported)
    unordered_map<string, const symbol*> symbol_table;
    for(const import_entry& import : imports)
    {
        const auto& imported_table = imported_tables.at(import.module);
        
        auto symbol_it = imported_table->find(import.identifier);
        if(symbol_it == imported_table->end())
            throw import_exception{boost::blank(), import_error::SYMBOL_NOT_FOUND};
            // TODO: no source location
        
        symbol_table[import.identifier] = &symbol_it->second;
        
        // insert import immediately in exports, if it is also exported
        auto exports_it = exports.find(import.identifier);
        if(exports_it != exports.end())
            exports_it->second = symbol_it->second; // TODO: expensive copy
    }
    

    bool import_exports_read = false;
    for(symbol& s : syntax_tree)
    {
        symbol::list* statement = s.cast_list();
        assert(statement != nullptr);
        
        if(is_import_export(*statement))
        {
            if(import_exports_read)
                throw import_exception{s.source, import_error::IMPORT_EXPORT_NOT_AT_BEGIN};
            else
                continue;
        }
        import_exports_read = true;

        if(statement->empty())
            throw eval_exception{s.source, eval_error::EMPTY_STATEMENT};

        symbol::reference* command = (*statement)[0].cast_reference();
        if(command == nullptr)
            throw eval_exception{(*statement)[0].source, eval_error::INVALID_COMMAND};
        if(command->identifier == "def")
        {
            if(statement->size() < 3)
                throw eval_exception{s.source, eval_error::DEF_TOO_FEW_ARGUMENTS};
            
            symbol::reference* defined_identifier = (*statement)[1].cast_reference();
            if(defined_identifier == nullptr)
                throw eval_exception{(*statement)[1].source, eval_error::INVALID_DEFINED};
            
            auto insertion_result = symbol_table.insert({defined_identifier->identifier, 0});
            if(!insertion_result.second) // no insertion took place, was already there
                throw eval_exception{(*statement)[1].source, eval_error::ALREADY_DEFINED};

            for(auto it = statement->begin() + 2; it != statement->end(); ++it)
                dispatch_references(*it, symbol_table);
            
            const symbol*& definition = insertion_result.first->second;
            if(statement->size() == 3)
                definition = &(*statement)[2];
            else // statement->size() > 3
            {
                assert(false); // macros not supported yet
            }

            auto exports_it = exports.find(defined_identifier->identifier);
            if(exports_it != exports.end())
                exports_it->second = *definition;
        }
        else
            assert(false); // not supported yet
    }

    // check whether all exports were defined
    for(auto& export_pair : exports)
    {
        if(!symbol_table.count(export_pair.first))
            throw import_exception{boost::blank(), import_error::EXPORT_UNDEFINED};
    }
}

