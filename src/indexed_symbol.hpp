#ifndef SYMBOL_INDEX_HPP_
#define SYMBOL_INDEX_HPP_

#include "symbol.hpp"

#include <boost/variant.hpp>

#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <memory>

typedef std::uint64_t symbol_index;

struct indexed_id;
struct indexed_lit;
struct indexed_ref;
struct indexed_list;
struct indexed_macro;
typedef std::pair<symbol_index, boost::variant<indexed_id, indexed_lit, indexed_ref, indexed_list, indexed_macro>> indexed_symbol;

struct indexed_id
{
    std::size_t id;
};
struct indexed_lit
{
    std::string str;
};
struct indexed_ref
{
    identifier_id_t identifier;
    symbol_index refered_index;
};
struct indexed_list
{
    std::vector<symbol_index> vec;
};
struct indexed_macro
{
};

std::size_t to_indexed_symbol_impl(const symbol& s, std::vector<indexed_symbol>& result);

// TODO: these 2 functions assume non-cyclic symbols
std::vector<indexed_symbol> to_indexed_symbol(list_symbol::const_iterator begin, list_symbol::const_iterator end);

any_symbol to_symbol_impl(symbol_index index, const std::vector<indexed_symbol>& indexed_symbols, std::vector<std::unique_ptr<any_symbol>>& symbol_store);

std::pair<any_symbol, std::vector<std::unique_ptr<any_symbol>>> to_symbol(symbol_index index, const std::vector<indexed_symbol>& indexed_symbols);

#endif

