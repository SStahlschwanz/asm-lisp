#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include <boost/variant.hpp>

#include <vector>
#include <exception>
#include <initializer_list>

#include "symbol_source.hpp"

class none_symbol;
class lit_symbol;
class ref_symbol;
class list_symbol;

class symbol_impl;

class symbol
{
public:
    enum type_value
    {
        NONE,
        LITERAL,
        REFERENCE,
        LIST
    };
    
    type_value type() const;
    const symbol_source& source() const;
    void source(const symbol_source& new_source);
    
    bool is_none() const;
    none_symbol& cast_none();
    const none_symbol& cast_none() const
    {
        return const_cast<symbol*>(this)->cast_none();
    }
    
    bool is_lit() const;
    lit_symbol& lit();
    const lit_symbol& lit() const
    {
        return const_cast<symbol*>(this)->lit();
    }
    lit_symbol& lit_else(const std::exception& exc)
    {
        if(type() == LITERAL)
            return lit();
        else
            throw exc;
    }
    const lit_symbol& lit_else(const std::exception& exc) const
    {
        return const_cast<symbol*>(this)->lit_else(exc);
    }

    bool is_ref() const;
    ref_symbol& ref();
    const ref_symbol& ref() const
    {
        return const_cast<symbol*>(this)->ref();
    }
    ref_symbol& ref_else(const std::exception& exc)
    {
        if(type() == REFERENCE)
            return ref();
        else
            throw exc;
    }
    const ref_symbol& ref_else(const std::exception& exc) const
    {
        return const_cast<symbol*>(this)->ref_else(exc);
    }
    
    bool is_list() const;
    list_symbol& list();
    const list_symbol& list() const
    {
        return const_cast<symbol*>(this)->list();
    }
    list_symbol& list_else(const std::exception& exc)
    {
        if(type() == LIST)
            return list();
        else
            throw exc;
    }
    const list_symbol& list_else(const std::exception& exc) const
    {
        return const_cast<symbol*>(this)->list_else(exc);
    }
    
    template<class Functor>
    void visit(Functor&& f)
    {
        switch(type())
        {
        case NONE:
            assert(type() != NONE); // crashes always
            break;
        case LITERAL:
            f(lit());
            break;
        case REFERENCE:
            f(ref());
            break;
        case LIST:
            f(list());
            break;
        default:
            assert(false);
            break;
        }
    }
    template<class Functor>
    void visit(Functor&& f) const
    {
        const_cast<symbol*>(this)->visit([&](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type T;
            f(const_cast<const T&>(obj));
        });
    }
    template<class Functor>
    void visit_none(Functor&& f)
    {
        switch(type())
        {
        case NONE:
            f(cast_none());
            break;
        case LITERAL:
            f(lit());
            break;
        case REFERENCE:
            f(ref());
            break;
        case LIST:
            f(list());
            break;
        default:
            assert(false);
            break;
        }
    }
    template<class Functor>
    void visit_none(Functor&& f) const
    {
        const_cast<symbol*>(this)->visit_none([&](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type T;
            f(const_cast<const T&>(obj));
        });
    }
private:
    friend class any_symbol;
    friend class symbol_impl;
    ~symbol()
    {}

    symbol_impl& impl();
    const symbol_impl& impl() const;
};

class symbol_impl
  : public symbol
{
private:
    friend class symbol;
    friend class none_symbol;
    friend class lit_symbol;
    friend class ref_symbol;
    friend class list_symbol;

    ~symbol_impl()
    {}
    type_value type_id;
    symbol_source source_value;
};

class none_symbol
  : public symbol_impl
{
public:
    none_symbol()
    {
        type_id = NONE;
    }

    bool operator==(const none_symbol&) const
    {
        return true;
    }
    bool operator!=(const none_symbol&) const
    {
        return false;
    }
};

class lit_symbol
  : public symbol_impl
{
public:
    lit_symbol(std::string str)
      : s(std::move(str))
    {
        type_id = LITERAL;
    }
    lit_symbol(const char* str)
      : lit_symbol(std::string{str})
    {}
    lit_symbol()
      : lit_symbol("")
    {}
    
    lit_symbol(const lit_symbol&) = default;
    lit_symbol(lit_symbol&& that) noexcept
      : lit_symbol(std::move(that.s))
    {}

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
  : public symbol_impl
{
public:
    ref_symbol(std::string str, const symbol* reference)
      : s(std::move(str))
    {
        type_id = REFERENCE;
        r = reference;
    }
    ref_symbol()
      : ref_symbol("", 0)
    {}

    ref_symbol(const ref_symbol&) = default;
    ref_symbol(ref_symbol&& that) noexcept
      : ref_symbol(std::move(that.s), that.r)
    {}

    explicit ref_symbol(std::string str)
      : ref_symbol(std::move(str), 0)
    {}
    explicit ref_symbol(const char* str)
      : ref_symbol(std::string{str})
    {}
    
    ref_symbol& operator=(ref_symbol that)
    {
        std::swap(s, that.s);
        std::swap(r, that.r);
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

    const symbol* refered() const
    {
        return r;
    }
    void refered(const symbol* new_reference)
    {
        r = new_reference;
    }

    bool operator==(const ref_symbol& that) const
    {
        return s == that.s && r == that.r;
    }
    bool operator!=(const ref_symbol& that) const
    {
        return !(*this == that);
    }
private:
    std::string s;
    const symbol* r;
};
static_assert(std::is_nothrow_move_constructible<ref_symbol>::value, "");

 
class any_symbol;
class list_symbol
  : public symbol_impl
{
public:
    list_symbol(std::vector<any_symbol> vec)
      : v(std::move(vec))
    {
        type_id = LIST;
    }
    list_symbol()
      : list_symbol(std::vector<any_symbol>{})
    {}
    list_symbol(std::initializer_list<any_symbol> l);
    
    list_symbol(const list_symbol&) = default;
    list_symbol(list_symbol&& that) noexcept
      : list_symbol(std::move(that.v))
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



inline symbol_impl& symbol::impl()
{
    return *static_cast<symbol_impl*>(this);
}
inline const symbol_impl& symbol::impl() const
{
    return *static_cast<const symbol_impl*>(this);
}
inline symbol::type_value symbol::type() const
{
    return impl().type_id;
}
inline const symbol_source& symbol::source() const
{
    return impl().source_value;
}
inline void symbol::source(const symbol_source& new_source)
{
    impl().source_value = new_source;
}
inline bool symbol::is_none() const
{
    return impl().type_id == NONE;
}
inline none_symbol& symbol::cast_none()
{
    assert(impl().type_id == NONE);
    return *static_cast<none_symbol*>(this);
}
inline bool symbol::is_lit() const
{
    return impl().type_id == LITERAL;
}
inline lit_symbol& symbol::lit()
{
    assert(is_lit());
    return *static_cast<lit_symbol*>(this);
}
inline bool symbol::is_ref() const
{
    return impl().type_id == REFERENCE;
}
inline ref_symbol& symbol::ref()
{
    assert(is_ref());
    return *static_cast<ref_symbol*>(this);
}
inline bool symbol::is_list() const
{
    return impl().type_id == LIST;
}
inline list_symbol& symbol::list()
{
    assert(is_list());
    return *static_cast<list_symbol*>(this);
}

inline bool operator==(const symbol& lhs, const symbol& rhs)
{
    if(lhs.type() != rhs.type())
        return false;
    
    switch(lhs.type())
    {
    case symbol::NONE:
        return lhs.cast_none() == rhs.cast_none();
    case symbol::LITERAL:
        return lhs.lit() == rhs.lit();
    case symbol::REFERENCE:
        return lhs.ref() == rhs.ref();
    case symbol::LIST:
        return lhs.list() == rhs.list();
    default:
        assert(false);
    }
}
inline bool operator!=(const symbol& lhs, const symbol& rhs)
{
    return !(lhs == rhs);
}


class any_symbol
{
public:
    any_symbol()
      : content(none_symbol{})
    {}
    any_symbol(lit_symbol literal)
      : content(std::move(literal))
    {}
    any_symbol(ref_symbol reference)
      : content(std::move(reference))
    {}
    any_symbol(list_symbol list)
      : content(std::move(list))
    {}
    any_symbol(symbol&& s)
    {
        s.visit_none([&](auto& obj)
        {
            content = std::move(obj);
        });
    }
    any_symbol(const symbol& s)
    {
        s.visit_none([&](auto obj)
        {
            content = std::move(obj);
        });
    }
private:
    struct address_getter_type
        : boost::static_visitor<>
    {
        typedef symbol* result_type;
        template<class T>
        symbol* operator()(T& obj) const
        {
            return &obj;
        }
    };
public:
    /*
    symbol* operator&()
    {
        return boost::apply_visitor(address_getter_type(), content);
    }
    const symbol* operator&() const
    {
        return &(*const_cast<any_symbol*>(this));
    }
    */
    operator symbol&()
    {
        return *boost::apply_visitor(address_getter_type(), content);
    }
    operator const symbol&() const
    {
        return *const_cast<any_symbol*>(this);
    }

    symbol::type_value type() const
    {
        return symbol::NONE;
    }
    
    bool is_lit() const
    {
        return static_cast<const symbol&>(*this).is_lit();
    }
    lit_symbol& lit()
    {
        return static_cast<symbol&>(*this).lit();
    }
    const lit_symbol& lit() const
    {
        return static_cast<const symbol&>(*this).lit();
    }
    lit_symbol& lit_else(const std::exception& exc)
    {
        return static_cast<symbol&>(*this).lit_else(exc);
    }
    const lit_symbol& lit_else(const std::exception& exc) const
    {
        return static_cast<const symbol&>(*this).lit_else(exc);
    }

    bool is_ref() const
    {
        return static_cast<const symbol&>(*this).is_ref();
    }
    ref_symbol& ref()
    {
        return static_cast<symbol&>(*this).ref();
    }
    const ref_symbol& ref() const
    {
        return static_cast<const symbol&>(*this).ref();
    }
    ref_symbol& ref_else(const std::exception& exc)
    {
        return static_cast<symbol&>(*this).ref_else(exc);
    }
    const ref_symbol& ref_else(const std::exception& exc) const
    {
        return static_cast<const symbol&>(*this).ref_else(exc);
    }

    bool is_list() const
    {
        return static_cast<const symbol&>(*this).is_list();
    }
    list_symbol& list()
    {
        return static_cast<symbol&>(*this).list();
    }
    const list_symbol& list() const
    {
        return static_cast<const symbol&>(*this).list();
    }
    list_symbol& list_else(const std::exception& exc)
    {
        return static_cast<symbol&>(*this).list_else(exc);
    }
    const list_symbol& list_else(const std::exception& exc) const
    {
        return static_cast<const symbol&>(*this).list_else(exc);
    }

    template<class Visitor>
    void visit_none(Visitor&& visitor)
    {
        static_cast<symbol&>(*this).visit_none(visitor);
    }
    template<class Visitor>
    void visit_none(Visitor&& visitor) const
    {
        static_cast<const symbol&>(*this).visit_none(visitor);
    }
    template<class Visitor>
    void visit(Visitor&& visitor)
    {
        static_cast<symbol&>(*this).visit(visitor);
    }
    template<class Visitor>
    void visit(Visitor&& visitor) const
    {
        static_cast<const symbol&>(*this).visit(visitor);
    }
private:
    boost::variant<none_symbol, lit_symbol, ref_symbol, list_symbol> content;
};

/*
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
        sizeof(none_symbol), sizeof(lit_symbol), sizeof(ref_symbol), sizeof(list_symbol));

class any_symbol
  : public symbol
{
public:
    any_symbol()
    {
        construct(none_symbol());
    }
    any_symbol(const symbol& that)
    {
        that.visit_none([&](auto obj)
        {
            construct(obj);
        });
    }
    any_symbol(symbol&& that) noexcept
    {
        that.visit_none([&](auto& obj)
        {
            construct(std::move(obj));
        });
    }

    any_symbol(lit_symbol literal)
    {
        construct(std::move(literal));
    }
    any_symbol(ref_symbol reference)
    {
        construct(std::move(reference));
    }
    any_symbol(list_symbol list)
    {
        construct(std::move(list));
    }
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
        visit_none([](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type T;
            obj.~T();
        });
    }
};
*/

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

#endif

