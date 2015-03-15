#ifndef COMPILE_UNIT_HPP_
#define COMPILE_UNIT_HPP_

#include "module.hpp"
#include "compilation_context.hpp"

#include <boost/filesystem.hpp>

#include <vector>
#include <string>
#include <tuple>



struct wrong_file_extension
{};
struct circular_dependency
{};
struct file_not_found
{};
struct io_error
{};

struct parsed_file
{
    list_node& syntax_tree;
    dynamic_graph graph_owner;
    module_header header;
};

std::vector<std::size_t> toposort(const std::vector<std::vector<std::size_t>>& graph);
parsed_file read_file(std::size_t file_id, const boost::filesystem::path& p);
std::vector<module> compile_unit(const std::vector<boost::filesystem::path>& paths, compilation_context& context);

#endif

