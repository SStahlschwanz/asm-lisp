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
        reference(std::string id = std::string(), const symbol* ref = nullptr)
          : identifier(std::move(id)),
            refered(ref)
        {}
        std::string identifier;
        const symbol* refered;

        bool operator==(const reference& that) const
        {
            if(identifier != that.identifier)
                return false;

            if(refered != nullptr && that.refered != nullptr)
                return *refered == *that.refered;
            else
                return refered == nullptr && that.refered == nullptr;
        }
    };
    typedef std::vector<symbol> list;
    
    boost::variant<boost::blank, source_range> source;
    boost::variant<literal, reference, list> content;
 
    literal* cast_literal()
    {
        return boost::get<literal>(&content);
    }
    reference* cast_reference()
    {
        return boost::get<reference>(&content);
    }
    list* cast_list()
    {
        return boost::get<list>(&content);
    }
    
    const literal* cast_literal() const
    {
        return boost::get<literal>(&content);
    }
    const reference* cast_reference() const
    {
        return boost::get<reference>(&content);
    }
    const list* cast_list() const
    {
        return boost::get<list>(&content);
    }

    bool operator==(const symbol& that) const
    {
        return content == that.content;
    }
    bool operator!=(const symbol& that) const
    {
        return !(*this == that);
    }
};

namespace symbol_building
{

inline symbol lit(std::string str)
{
    return symbol{boost::blank(), std::move(str)};
}
inline symbol ref(std::string identifier, const symbol* refered = nullptr)
{
    return symbol{boost::blank(), symbol::reference(std::move(identifier), refered)};
}

template<class... Symbols>
symbol list(Symbols&&... symbols)
{
    return symbol{boost::blank(), symbol::list{std::forward<Symbols>(symbols)...}};
}

}

#endif

