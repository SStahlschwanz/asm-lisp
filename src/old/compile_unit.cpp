#include "compile_unit.hpp"

#include "module.hpp"
#include "compilation_context.hpp"
#include "error/compile_exception.hpp"
#include "error/import_export_error.hpp"

#include <boost/optional.hpp>

#include <utility>
#include <fstream>
#include <iterator>
#include <algorithm>

using std::vector;
using std::pair;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;
using std::move;
using std::find;
using std::string;

using boost::filesystem::path;
using boost::filesystem::exists;
using boost::optional;
using boost::none;

using namespace import_export_error;

vector<size_t> toposort(const vector<vector<size_t>>& graph)
{
    // crude implementation of topological sort
    vector<optional<vector<size_t>>> remaining_graph{graph.begin(), graph.end()};
    auto remove_node = [&](size_t node_to_remove)
    {
        remaining_graph[node_to_remove] = none;
        for(optional<vector<size_t>>& neighbors : remaining_graph)
        {
            if(neighbors)
                neighbors->erase(remove(neighbors->begin(), neighbors->end(), node_to_remove), neighbors->end());
        }
    };

    vector<size_t> result;
    result.reserve(graph.size());

    while(result.size() != graph.size())
    {
        for(size_t node = 0; node != remaining_graph.size(); ++node)
        {
            optional<vector<size_t>>& neighbors = remaining_graph[node];
            if(!neighbors)
                continue;
            if(neighbors->empty())
            {
                result.push_back(node);
                remove_node(node);
                goto node_removed;
            }
        }
        throw circular_dependency{};

        node_removed:;
    }
    
    return result;
}

vector<pair<list_symbol, module_header>> read_files(const vector<path>& paths, compilation_context& context)
{
    vector<pair<list_symbol, module_header>> result;
    result.reserve(paths.size());
    size_t file_id = 0;
    for(const path& p : paths)
    {
        if(p.extension() != ".al")
            throw wrong_file_extension{};
        if(!exists(p))
            throw file_not_found{};

        ifstream file{p.native(), ios::binary};
        if(!file)
            throw io_error{};
        istreambuf_iterator<char> begin{file};
        istreambuf_iterator<char> end{};
        parse_state<istreambuf_iterator<char>> state{begin, end, file_id, context};
        list_symbol syntax_tree = parse_file(state);
        module_header header = read_module_header(syntax_tree);

        result.push_back({move(syntax_tree), move(header)});

        ++file_id;
    }
    return result;
}

vector<module> compile_unit(const vector<path>& paths, compilation_context& context)
{
    vector<pair<list_symbol, module_header>> parsed_files = read_files(paths, context);
    assert(parsed_files.size() == paths.size());
    auto lookup_file_id = [&](const path& parent_path, const import_statement& import) -> size_t
    {
        path module_path{parent_path / ((string)import.imported_module + ".al")};
        auto it = find(paths.begin(), paths.end(), module_path);
        if(it == paths.end())
            fatal<id("module_not_found")>(import.imported_module.source());
        return it - paths.begin();
    };
    
    vector<vector<size_t>> dependency_graph{paths.size()};
    
    for(size_t file_id = 0; file_id != parsed_files.size(); ++file_id)
    {
        const pair<list_symbol, module_header>& read_file = parsed_files[file_id];
        const path& p = paths[file_id];
        path parent = p.parent_path();
        for(const import_statement& import : read_file.second.imports)
        {
            if((string)import.imported_module != "core")
                dependency_graph[file_id].push_back(lookup_file_id(parent, import)); 
        }
    }

    vector<size_t> compilation_order = toposort(dependency_graph);

    vector<module> modules;
    modules.reserve(parsed_files.size());
    for(size_t file_id : compilation_order)
    {
        path parent = paths[file_id].parent_path();
        auto lookup_module = [&](const import_statement& import) -> module&
        {
            if(import.imported_module == "core")
                return context.core_module();
            size_t file_id = lookup_file_id(parent, import);
            assert(file_id < modules.size());
            return modules[file_id];
        };

        list_symbol& syntax_tree = parsed_files[file_id].first;
        const module_header& header = parsed_files[file_id].second;
        module m = evaluate_module(move(syntax_tree), header, lookup_module, context);
        modules.push_back(move(m));
    }

    return modules;
}

