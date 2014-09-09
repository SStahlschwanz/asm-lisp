#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include <boost/variant.hpp>

#include "source_location.hpp"

class symbol
{
public:
    typedef std::string literal;
    struct reference
    {
        reference()
          : refered(nullptr)
        {}
        std::string identifier;
        symbol* refered;
    };
    typedef std::vector<symbol> list;
    
    boost::variant<boost::blank, source_range> source;
    boost::variant<literal, reference, list> content;


};

namespace symbol_building
{

inline symbol lit(std::string str)
{
    return symbol{boost::blank(), std::move(str)};
}
inline symbol ref(std::string identifier)
{
    return symbol{boost::blank(), std::move(identifier)};
}
inline symbol list(symbol::list list)
{
    return symbol{boost::blank(), std::move(list)};
}

}

#endif

