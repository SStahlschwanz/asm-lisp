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
using std::tuple;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;
using std::move;
using std::find;
using std::string;
using std::size_t;

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


parsed_file read_file(size_t file_id, const path& p)
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

    dynamic_graph graph_owner;
    parse_state<istreambuf_iterator<char>> state{begin, end, file_id, graph_owner};
    list_node& syntax_tree = parse_file(state);
    module_header header = read_module_header(syntax_tree);

    return {syntax_tree, move(graph_owner), move(header)};
}

vector<module> compile_unit(const vector<path>& paths, compilation_context& context)
{
    auto parsed_files = save<vector<parsed_file>>(mapped(enumerate(paths), unpacking(
    [&](size_t index, const path& p) -> parsed_file
    {
        return read_file(index, p);
    })));

    auto lookup_file_id = [&](const path& parent_path, const import_statement& import) -> size_t
    {
        path module_path{parent_path / (save<string>(rangeify(import.imported_module)) + ".al")};
        auto it = find(paths.begin(), paths.end(), module_path);
        if(it == paths.end())
            fatal<id("module_not_found")>(import.imported_module.source());
        return it - paths.begin();
    };
    
    auto dependency_graph = save<vector<vector<size_t>>>(mapped(zipped(paths, parsed_files), unpacking(
    [&](const path& p, parsed_file& f)
    {
        path parent = p.parent_path();
        auto& imports = f.header.imports;
        auto non_core_imports = filtered(imports,
        [&](const import_statement& import)
        {
            static const string core_str = "core";
            return rangeify(import.imported_module) != rangeify(core_str);
        });

        return save<vector<size_t>>(mapped(non_core_imports,
        [&](const import_statement& import)
        {
            return lookup_file_id(parent, import);
        }));
    })));

    vector<size_t> compilation_order_indices = toposort(dependency_graph);
    auto ordered_paths = permuted(paths, compilation_order_indices);
    auto ordered_parsed_files = permuted(parsed_files, compilation_order_indices);

    vector<module> modules;
    modules.reserve(paths.size());
    for_each(zipped(ordered_paths, ordered_parsed_files), unpacking(
    [&](const path& p, parsed_file& file)
    {
        path parent = p.parent_path();
        auto lookup_module = [&](const import_statement& import) -> module&
        {
            const static string core_str = "core";
            if(rangeify(import.imported_module) == rangeify(core_str))
                return context.core_module();
            size_t file_id = lookup_file_id(parent, import);
            assert(file_id < modules.size());
            return modules[file_id];
        };

        modules.emplace_back(evaluate_module(file.syntax_tree, move(file.graph_owner), move(file.header), lookup_module));
    }));

    return modules;
}

