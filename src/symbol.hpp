#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include <boost/variant.hpp>

#include <vector>
#include <initializer_list>
#include <memory>

#include "symbol_source.hpp"
#include "boost_variant_utils.hpp"

class id_symbol;
class lit_symbol;
class ref_symbol;
class list_symbol;
class macro_symbol;

class any_symbol;

typedef std::size_t identifier_id_t;

namespace symbol_detail
{

class symbol_impl;

}

namespace symbol_shortcuts
{

typedef lit_symbol lit;
typedef ref_symbol ref;
typedef list_symbol list;
typedef macro_symbol macro;

}

class symbol
{
public:
    enum type_value
    {
        ID,
        LITERAL,
        REFERENCE,
        LIST,
        MACRO
    };
    
    type_value type() const;
    const symbol_source& source() const;
    void source(const symbol_source& new_source);
    
    template<class SymbolType>
    bool is() const;
    template<class SymbolType>
    SymbolType& cast();
    template<class SymbolType>
    const SymbolType& cast() const
    {
        return const_cast<symbol*>(this)->cast<SymbolType>();
    }
    template<class SymbolType, class FunctorType>
    SymbolType& cast_else(FunctorType&& functor)
    {
        if(is<SymbolType>())
            return cast<SymbolType>();
        else
        {
            functor();
            assert(false);
            return *static_cast<SymbolType*>(nullptr); // to suppress warnings
        }
    }
    template<class SymbolType, class FunctorType>
    const SymbolType& cast_else(FunctorType&& functor) const
    {
        return const_cast<symbol*>(this)->cast_else<SymbolType>(
                std::forward<FunctorType>(functor));
    }

    template<class ResultType = void, class FunctorType>
    ResultType visit(FunctorType&& functor);
    template<class ResultType = void, class FunctorType>
    ResultType visit(FunctorType&& functor) const
    {
        return const_cast<symbol*>(this)->visit<ResultType>([&](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type actual_type;
            functor(const_cast<const actual_type&>(obj));
        });
    }
    template<class ResultType = void, class Functor1Type, class Functor2Type, class... FunctorTypes>
    ResultType visit(Functor1Type&& functor1, Functor2Type&& functor2, FunctorTypes&&... functors)
    {
        boost_variant_utils_detail::lambda_visitor<ResultType, Functor1Type, Functor2Type, FunctorTypes...> visitor{functor1, functor2, functors...};
        return visit<ResultType>(visitor);
    }
    template<class ResultType = void, class Functor1Type, class Functor2Type, class... FunctorTypes>
    ResultType visit(Functor1Type&& functor1, Functor2Type&& functor2, FunctorTypes&&... functors) const
    {
        boost_variant_utils_detail::lambda_visitor<ResultType, Functor1Type, Functor2Type, FunctorTypes...> visitor{functor1, functor2, functors...};
        return visit<ResultType>(visitor);
    }
private:
    friend class any_symbol;
    friend class symbol_detail::symbol_impl;
    ~symbol()
    {}

    symbol_detail::symbol_impl& impl();
    const symbol_detail::symbol_impl& impl() const;
};

namespace symbol_detail
{
class symbol_impl
  : public symbol
{
private:
    friend class ::symbol;
    friend class ::id_symbol;
    friend class ::lit_symbol;
    friend class ::ref_symbol;
    friend class ::list_symbol;
    friend class ::macro_symbol;

    symbol_impl(type_value type_val)
      : t(type_val)
    {}
    ~symbol_impl() = default;

    type_value t;
    symbol_source source_value;
};
}
class id_symbol
  : public symbol_detail::symbol_impl
{
public:
    static constexpr type_value type_id = ID;

    id_symbol(size_t new_id)
      : symbol_detail::symbol_impl(type_id),
        i(new_id)
    {}
    id_symbol()
      : id_symbol(0)
    {}
    bool operator==(const id_symbol& that) const
    {
        return i == that.i;
    }
    bool operator!=(const id_symbol& that) const
    {
        return !(*this == that);
    }

    size_t id() const
    {
        return i;
    }
    void id(size_t new_id)
    {
        i = new_id;
    }
private:
    size_t i;
};

class lit_symbol
  : public symbol_detail::symbol_impl
{
public:
    static constexpr type_value type_id = LITERAL;

    lit_symbol(std::string str)
      : symbol_detail::symbol_impl(type_id),
        s(std::move(str))
    {}
    lit_symbol(const char* str)
      : lit_symbol(std::string{str})
    {}
    lit_symbol()
      : lit_symbol("")
    {}
    
    lit_symbol(lit_symbol&& that) noexcept
      : symbol_detail::symbol_impl(std::move(that)),
        s(std::move(that.s))
    {}
    lit_symbol(const lit_symbol&) = default;

    lit_symbol& operator=(lit_symbol that)
    {
        std::swap(s, that.s);
        return *this;
    }
    
    typedef std::string::iterator iterator;
    typedef std::string::const_iterator const_iterator;
    iterator begin()
    {
        return s.begin();
    }
    iterator end()
    {
        return s.end();
    }
    const_iterator begin() const
    {
        return s.begin();
    }
    const_iterator end() const
    {
        return s.end();
    }
    char& operator[](size_t i)
    {
        return s[i];
    }
    const char& operator[](size_t i) const
    {
        return s[i];
    }
    void push_back(char c)
    {
        s.push_back(c);
    }
    void pop_back()
    {
        s.pop_back();
    }
    bool empty() const
    {
        return s.empty();
    }

    bool operator==(const lit_symbol& that) const
    {
        return s == that.s;
    }
    bool operator!=(const lit_symbol& that) const
    {
        return !(*this == that);
    }

private:
    std::string s;
};
static_assert(std::is_nothrow_move_constructible<lit_symbol>::value, "");

class ref_symbol
  : public symbol_detail::symbol_impl
{
public:
    static constexpr type_value type_id = REFERENCE;
    ref_symbol(identifier_id_t identifier_id, const symbol* reference)
      : symbol_detail::symbol_impl(type_id),
        identifier_id(identifier_id),
        r(reference)
    {}
#ifndef NDEBUG
    ref_symbol(identifier_id_t identifier_id, std::string str, const symbol* reference)
      : symbol_detail::symbol_impl(type_id),
        s(std::move(str)),
        identifier_id(identifier_id),
        r(reference)
    {}
#endif

    ref_symbol(const ref_symbol&) = default;
    ref_symbol(ref_symbol&& that) noexcept
      : symbol_detail::symbol_impl(std::move(that)),
        identifier_id(that.identifier_id),
        r(that.r)
    {}

    explicit ref_symbol(identifier_id_t identifier_id)
      : ref_symbol(identifier_id, nullptr)
    {}
    ref_symbol& operator=(ref_symbol that)
    {
        std::swap(identifier_id, that.identifier_id);
        std::swap(r, that.r);
        return *this;
    }
    
    const symbol* refered() const
    {
        return r;
    }
    void refered(const symbol* new_reference)
    {
        r = new_reference;
    }
    identifier_id_t identifier() const
    {
        return identifier_id;
    }
    void identifier(identifier_id_t new_identifier_id)
    {
        identifier_id = new_identifier_id;
    }

    bool operator==(const ref_symbol& that) const
    {
        return identifier_id == that.identifier_id && r == that.r;
    }
    bool operator!=(const ref_symbol& that) const
    {
        return !(*this == that);
    }

private:
#ifndef NDEBUG
    std::string s;
#endif
    identifier_id_t identifier_id;
    const symbol* r;
};
static_assert(std::is_nothrow_move_constructible<ref_symbol>::value, "");

 
class list_symbol
  : public symbol_detail::symbol_impl
{
public:
    static constexpr type_value type_id = LIST;
    list_symbol(std::vector<any_symbol> vec)
      : symbol_detail::symbol_impl(type_id),
        v(std::move(vec))
    {}
    list_symbol()
      : list_symbol(std::vector<any_symbol>{})
    {}
    list_symbol(std::initializer_list<any_symbol> l);
    
    list_symbol(const list_symbol&) = default;
    list_symbol(list_symbol&& that) noexcept
      : symbol_detail::symbol_impl(std::move(that)),
        v(std::move(that.v))
    {}

    list_symbol& operator=(list_symbol that)
    {
        std::swap(v, that.v);
        return *this;
    }

    typedef std::vector<any_symbol>::iterator iterator;
    typedef std::vector<any_symbol>::const_iterator const_iterator;
    iterator begin()
    {
        return v.begin();
    }
    iterator end()
    {
        return v.end();
    }
    const_iterator begin() const
    {
        return v.begin();
    }
    const_iterator end() const
    {
        return v.end();
    }
    any_symbol& operator[](size_t i);
    const any_symbol& operator[](size_t i) const;
    void push_back(symbol&& s);
    void push_back(const symbol& s);
    void pop_back()
    {
        v.pop_back();
    }
    size_t size() const
    {
        return v.size();
    }
    bool empty() const
    {
        return v.empty();
    }

    bool operator==(const list_symbol& that) const
    {
        return v == that.v;
    }
    bool operator!=(const list_symbol& that) const
    {
        return !(*this == that);
    }

private:
    std::vector<any_symbol> v;
};
static_assert(std::is_nothrow_move_constructible<list_symbol>::value, "");

class macro_symbol
  : public symbol_detail::symbol_impl
{
public:
    static constexpr type_value type_id = MACRO;

    typedef std::function<any_symbol (list_symbol::const_iterator begin, list_symbol::const_iterator end)> macro_function;

    macro_symbol(macro_function func)
      : symbol_detail::symbol_impl(MACRO),
        f(std::move(func))
    {}
    macro_symbol(const macro_symbol&) = default;
    macro_symbol(macro_symbol&& that) noexcept
      : macro_symbol(std::move(that.f))
    {}
    
    any_symbol operator()(list_symbol::const_iterator begin, list_symbol::const_iterator end) const;
    bool operator==(const macro_symbol& that) const
    {
        // TODO
        assert(false);
    }
    bool operator!=(const macro_symbol& that) const
    {
        return !(*this == that);
    }
private:
    macro_function f;
};
static_assert(std::is_nothrow_move_constructible<macro_symbol>::value, "");

inline symbol_detail::symbol_impl& symbol::impl()
{
    return *static_cast<symbol_detail::symbol_impl*>(this);
}
inline const symbol_detail::symbol_impl& symbol::impl() const
{
    return *static_cast<const symbol_detail::symbol_impl*>(this);
}
inline symbol::type_value symbol::type() const
{
    return impl().t;
}
inline const symbol_source& symbol::source() const
{
    return impl().source_value;
}
inline void symbol::source(const symbol_source& new_source)
{
    impl().source_value = new_source;
}
template<class SymbolType>
inline bool symbol::is() const
{
    return type() == SymbolType::type_id;
}
template<class SymbolType>
SymbolType& symbol::cast()
{
    assert(is<SymbolType>());
    return *static_cast<SymbolType*>(this);
}
template<class ResultType, class FunctorType>
ResultType symbol::visit(FunctorType&& f)
{
    switch(type())
    {
    case ID:
        return f(cast<id_symbol>());
        break;
    case LITERAL:
        return f(cast<lit_symbol>());
        break;
    case REFERENCE:
        return f(cast<ref_symbol>());
        break;
    case LIST:
        return f(cast<list_symbol>());
        break;
    case MACRO:
        return f(cast<macro_symbol>());
        break;
    }
}


inline bool operator==(const symbol& lhs, const symbol& rhs)
{
    if(lhs.type() != rhs.type())
        return false;
    
    switch(lhs.type())
    {
    case symbol::ID:
        return lhs.cast<id_symbol>() == rhs.cast<id_symbol>();
    case symbol::LITERAL:
        return lhs.cast<lit_symbol>() == rhs.cast<lit_symbol>();
    case symbol::REFERENCE:
        return lhs.cast<ref_symbol>() == rhs.cast<ref_symbol>();
    case symbol::LIST:
        return lhs.cast<list_symbol>() == rhs.cast<list_symbol>();
    case symbol::MACRO:
        return lhs.cast<macro_symbol>() == rhs.cast<macro_symbol>();
    }
}
inline bool operator!=(const symbol& lhs, const symbol& rhs)
{
    return !(lhs == rhs);
}


constexpr size_t constexpr_max_for_symbol(size_t a, size_t b)
{
    if(a > b)
        return a;
    else
        return b;
}
constexpr size_t constexpr_max_for_symbol(size_t a)
{
    return a;
}
template <class... TS>
constexpr size_t constexpr_max_for_symbol(size_t a, size_t b, TS... s)
{
    return constexpr_max_for_symbol(a, constexpr_max_for_symbol(b, s...));
}

constexpr const size_t max_symbol_size = constexpr_max_for_symbol(
        sizeof(lit_symbol), sizeof(ref_symbol),
        sizeof(list_symbol), sizeof(macro_symbol),
        sizeof(id_symbol));

class any_symbol
  : public symbol
{
public:
    any_symbol(const symbol& that)
    {
        that.visit([&](auto obj)
        {
            construct(obj);
        });
    }
    any_symbol(symbol&& that) noexcept
    {
        that.visit([&](auto& obj)
        {
            construct(std::move(obj));
        });
    }
    any_symbol(const any_symbol& that)
      : any_symbol(static_cast<const symbol&>(that))
    {}
    any_symbol(any_symbol&& that)
      : any_symbol(static_cast<symbol&&>(that))
    {}
    
    ~any_symbol() noexcept
    {
        destruct();
    }
    
    any_symbol& operator=(symbol&& that) noexcept
    {
        destruct();
        that.visit([&](auto& obj)
        {
            construct(std::move(obj));
        });
        return *this;
    }
    any_symbol& operator=(const symbol& that)
    {
        that.visit([&](auto obj)
        {
            // obj has been copied already
            destruct(); // noexcept
            construct(std::move(obj)); // noexcept
        });
        return *this;
    }
    any_symbol& operator=(any_symbol that)
    {
        return (*this = static_cast<symbol&&>(that));
    }
private:
    char buffer[max_symbol_size];
    
    template<class T>
    void construct(T&& obj)
    {
        typedef typename std::decay<T>::type actual_type;
        new(buffer) actual_type(std::forward<T>(obj));
    }
    void destruct() noexcept
    {
        visit([](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type T;
            obj.~T();
        });
    }
};



inline list_symbol::list_symbol(std::initializer_list<any_symbol> l)
  : list_symbol(std::vector<any_symbol>{l.begin(), l.end()})
{}
inline any_symbol& list_symbol::operator[](size_t i)
{
    return v[i];
}
inline const any_symbol& list_symbol::operator[](size_t i) const
{
    return v[i];
}
inline void list_symbol::push_back(symbol&& s)
{
    v.push_back(std::move(s));
}
inline void list_symbol::push_back(const symbol& s)
{
    v.push_back(s);
}

inline any_symbol macro_symbol::operator()(list_symbol::const_iterator begin, list_symbol::const_iterator end) const
{
    return f(begin, end);
}

#endif

