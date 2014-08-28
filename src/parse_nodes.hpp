#ifndef PARSE_NODES_HPP_
#define PARSE_NODES_HPP_

#include "symbol.hpp"

#include <boost/optional.hpp>

template <class State>
boost::optional<Symbol> parse_node(State& state);

template <class State>
boost::optional<symbol> parse_nodes(State& state)
{

}

#endif

