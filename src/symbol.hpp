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
        const symbol* refered;
    };
    typedef std::vector<symbol> list;
    
    boost::variant<boost::blank, source_range> source;
    boost::variant<literal, reference, list> content;
 
    literal* cast_literal()
    {
        return boost::get<literal*>(content);
    }
    reference* cast_reference()
    {
        return boost::get<reference*>(content);
    }
    list* cast_list()
    {
        return boost::get<list*>(content);
    }
    
    const literal* cast_literal() const
    {
        return boost::get<literal*>(content);
    }
    const reference* cast_reference() const
    {
        return boost::get<reference*>(content);
    }
    const list* cast_list() const
    {
        return boost::get<list*>(content);
    }
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

