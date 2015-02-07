#include "module.hpp"

#include "parse_state.hpp"
#include "parse.hpp"
#include "import_error.hpp"
#include "define_error.hpp"

#include <fstream>
#include <stdexcept>
#include <stack>

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

module read_module(std::istream& stream)
{
    istreambuf_iterator<char> begin(stream);
    istreambuf_iterator<char> end;
    
    module result;
    
    parse_state<istreambuf_iterator<char>> state(begin, end);
    result.syntax_tree = parse_file(state);
    
    result.required_modules = required_modules(result.syntax_tree);

    return result;
}

template<class LookupFunctor>
void dispatch_references(symbol& root_node, LookupFunctor&& lookup)
{
    if(symbol::reference* r = root_node.cast_reference())
        r->refered = lookup(r->identifier);
    else if(symbol::list* l = root_node.cast_list())
    {
        for(symbol& child : *l)
        {
            dispatch_references(child, lookup);
        }
    }
    // ignore literal, nothing to dispatch
}

void dispatch_and_eval(module& m, const std::vector<const module*>& dependencies)
{
    stack<symbol::list*> open_nodes;

    auto lookup_symbol = [&](const string& identifier) -> const symbol*
    {
        const symbol* result = nullptr;
        auto it = m.defined_symbols.find(identifier);
        if(it != m.defined_symbols.end())
            result = &it->second;

        for(const module* mod : dependencies)
        {
            auto it = mod->defined_symbols.find(identifier);
            if(it != mod->defined_symbols.end())
            {
                if(result != nullptr)
                    throw import_error("reference is ambiguous: " + identifier);
                else if(result != nullptr)
                    result = &it->second;
            }
        }

        return result;
    };
    
    for(symbol& s : m.syntax_tree)
    {
        symbol::list* l = s.cast_list();
        assert(l != nullptr);
        if(symbol::reference* command = l->front().cast_reference())
        {
            if(command->identifier == "def")
            {
                if(l->size() < 3)
                    throw define_error("\"def\" needs as least 3 arguments");
                symbol::reference* arg1 = (*l)[1].cast_reference();
                if(arg1 == nullptr)
                    throw define_error("\"def\": first argument is not identifier");
                if(m.defined_symbols.find(arg1->identifier) != m.defined_symbols.end())
                    throw define_error("\"def\": redefinition of symbol");
                if(arg1->identifier == "def")
                    throw define_error("\"def\": forbidden identifier \"def\"");
                
                for(auto it = l->begin() + 2; it != l->end(); ++it)
                    dispatch_references(*it, lookup_symbol);
                
                if(l->size() == 3)
                    m.defined_symbols[arg1->identifier] = (*l)[2];
                else // l->size() > 3
                {
                    symbol::list defined_list(l->begin() + 2, l->end());
                    m.defined_symbols[arg1->identifier] = symbol{boost::blank(), std::move(defined_list)};
                    //symbol::reference* macro_ref = (*l)[2].cast_reference();
                    //if(macro_ref == nullptr)
                    //    throw define_error("\def\": need identifier here");
                    //if(macro_ref->refered == nullptr)
                    //    throw define_error("\def\": use of undefined error");
                    
                }
            }
        }
    }
}
