#ifndef SYMBOL_BUILDING_HPP_
#define SYMBOL_BUILDING_HPP_

#include "../src/symbol.hpp"

inline symbol lit(std::string str)
{
    return symbol{std::move(str), boost::blank()};
}
inline symbol sref(std::string identifier, const symbol* refered = nullptr)
{
    return symbol{symbol::reference(std::move(identifier), refered), boost::blank()};
}

template<class... Symbols>
symbol list(Symbols&&... symbols)
{
    return symbol{symbol::list{std::forward<Symbols>(symbols)...}, boost::blank()};
}

#endif

