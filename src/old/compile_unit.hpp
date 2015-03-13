#ifndef COMPILE_UNIT_HPP_
#define COMPILE_UNIT_HPP_

#include "module.hpp"
#include "compilation_context.hpp"
#include "symbol.hpp"

#include <boost/filesystem.hpp>

#include <vector>
#include <string>



struct wrong_file_extension
{};
struct circular_dependency
{};
struct file_not_found
{};
struct io_error
{};

std::vector<std::size_t> toposort(const std::vector<std::vector<std::size_t>>& graph);
std::vector<std::pair<list_symbol, module_header>> read_files(const std::vector<boost::filesystem::path>& paths, compilation_context& context);
std::vector<module> compile_unit(const std::vector<boost::filesystem::path>& paths, compilation_context& context);

#endif

