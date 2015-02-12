#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include "source_location.hpp"

#include <boost/variant.hpp>

#include <functional>

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
    struct macro
    {
        std::function<symbol (symbol::list param)> f;
        bool operator==(const macro&) const
        {
            return true;
        }
    };
    
    typedef boost::variant<boost::blank, source_range> source_type;
    source_type source;
    typedef boost::variant<literal, reference, list, macro> content_type;
    content_type content;
    
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
    macro* cast_macro()
    {
        return boost::get<macro>(&content);
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
    const macro* cast_macro() const
    {
        return boost::get<macro>(&content);
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
inline symbol sref(std::string identifier, const symbol* refered = nullptr)
{
    return symbol{boost::blank(), symbol::reference(std::move(identifier), refered)};
}

template<class... Symbols>
symbol list(Symbols&&... symbols)
{
    return symbol{boost::blank(), symbol::list({std::forward<Symbols>(symbols)...})};
}

}

#endif

