#include "compilation_unit.hpp"

#include "import_error.hpp"

#include <unordered_set>
#include <fstream>
#include <iterator>
/*
using namespace std;

vector<string> compilation_order(const unordered_map<string, module> modules)
{
    unordered_map<string, unordered_set<string>> dependency_graph;
    
    for(const std::pair<const string, module> p : modules)
    {
        unordered_set<string>& dependencies = dependency_graph[p.first];
        for(const string& dependency : p.second.required_modules)
        {
            if(modules.count(dependency) == 0)
                throw import_error("invalid module: " + dependency);
            dependencies.insert(dependency);
        }
    }
    
    vector<string> order;
    while(!dependency_graph.empty())
    {
        for(std::pair<const string, unordered_set<string>>& node : dependency_graph)
        {
            if(node.second.empty())
            {
                order.push_back(node.first);
                for(std::pair<const string, unordered_set<string>>& other_node : dependency_graph)
                    other_node.second.erase(node.first);
                dependency_graph.erase(node.first);
                goto free_node_found;
            }
        }
        throw import_error("cyclic dependencies detected");   
        free_node_found:;
    }
    return order;
}

std::unordered_map<std::string, symbol> create_core_symbol_table()
{
    std::unordered_map<std::string, symbol> result;
    return result;
}

const auto core_symbol_table = create_core_symbol_table();

compilation_unit compile(const vector<string>& files)
{
    compilation_unit unit;
    
    for(const string& file_name : files)
    {
        ifstream stream(file_name);
        if(stream)
        {
            istream_iterator<char> begin{stream};
            istream_iterator<char> end{};
            parse_state<istream_iterator<char>> state{begin, end};
            unit.modules[file_name] = read_module(state);
        }
        else
            throw std::runtime_error("error reading file: " + file_name);
    }
    
    vector<string> order = compilation_order(unit.modules);
    
    for(const string& module_name : order)
    {
        module& m = unit.modules[module_name];
        std::vector<const std::unordered_map<std::string, symbol>*> imported_tables;
        for(const string& dependency : m.required_modules)
        {
            if(dependency == "core")
                imported_tables.push_back(&core_symbol_table);
            else
            {
                auto module_it = unit.modules.find(dependency);
                if(module_it == unit.modules.end())
                    throw std::runtime_error("invalid import: " + dependency);
                imported_tables.push_back(&module_it->second.defined_symbols);
            }
        }
        dispatch_and_eval(m, imported_tables);
    }

    return unit;
}
*/
