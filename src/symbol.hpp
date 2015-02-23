#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include <boost/variant.hpp>

#include <vector>
#include <exception>
#include <initializer_list>

#include "symbol_source.hpp"

class none_symbol;
class id_symbol;
class lit_symbol;
class ref_symbol;
class owning_ref_symbol;
class list_symbol;
class macro_symbol;

class symbol_impl;

class symbol
{
public:
    enum type_value
    {
        NONE,
        ID,
        LITERAL,
        OWNING_REFERENCE,
        REFERENCE,
        LIST,
        MACRO
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
    
    bool is_id() const;
    id_symbol& cast_id();
    const id_symbol& cast_id() const
    {
        return const_cast<symbol*>(this)->cast_id();
    }
    id_symbol& cast_id_else(const std::exception& exc)
    {
        if(type() == ID)
            return cast_id();
        else
            throw exc;
    }
    const id_symbol& cast_id_else(const std::exception& exc) const
    {
        return const_cast<symbol*>(this)->cast_id_else(exc);
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
    
    bool is_owning_ref() const;
    owning_ref_symbol& owning_ref();
    const owning_ref_symbol& owning_ref() const
    {
        return const_cast<symbol*>(this)->owning_ref();
    }
    owning_ref_symbol& owning_ref_else(const std::exception& exc)
    {
        if(type() == REFERENCE)
            return owning_ref();
        else
            throw exc;
    }
    const owning_ref_symbol& owning_ref_else(const std::exception& exc) const
    {
        return const_cast<symbol*>(this)->owning_ref_else(exc);
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
    bool is_macro() const;
    macro_symbol& macro();
    const macro_symbol& macro() const
    {
        return const_cast<symbol*>(this)->macro();
    }
    macro_symbol& macro_else(const std::exception& exc)
    {
        if(type() == MACRO)
            return macro();
        else
            throw exc;
    }
    const macro_symbol& macro_else(const std::exception& exc) const
    {
        return const_cast<symbol*>(this)->macro_else(exc);
    }
    
    template<class Functor>
    void visit(Functor&& f)
    {
        switch(type())
        {
        case NONE:
            assert(type() != NONE); // crashes always
            break;
        case ID:
            f(cast_id());
            break;
        case LITERAL:
            f(lit());
            break;
        case OWNING_REFERENCE:
            f(owning_ref());
            break;
        case REFERENCE:
            f(ref());
            break;
        case LIST:
            f(list());
            break;
        case MACRO:
            f(macro());
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
        case ID:
            f(cast_id());
            break;
        case LITERAL:
            f(lit());
            break;
        case OWNING_REFERENCE:
            f(owning_ref());
            break;
        case REFERENCE:
            f(ref());
            break;
        case LIST:
            f(list());
            break;
        case MACRO:
            f(macro());
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
    friend class id_symbol;
    friend class lit_symbol;
    friend class owning_ref_symbol;
    friend class ref_symbol;
    friend class list_symbol;
    friend class macro_symbol;

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

class id_symbol
  : public symbol_impl
{
public:
    id_symbol(size_t new_id)
      : i(new_id)
    {
        type_id = ID;
    }
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

class any_symbol;
class owning_ref_symbol
  : public symbol_impl
{
public:
    owning_ref_symbol(std::string str, std::unique_ptr<any_symbol> reference)
      : s(std::move(str)),
        r(std::move(reference))
    {
        type_id = OWNING_REFERENCE;
    }
    owning_ref_symbol()
      : owning_ref_symbol("", 0)
    {}

    owning_ref_symbol(const owning_ref_symbol& that)
      : s(that.s)
    {
        if(that.r)
            r = std::make_unique<any_symbol>(*that.r);
        else
            r = nullptr;
    }
    owning_ref_symbol(owning_ref_symbol&& that) noexcept
      : owning_ref_symbol(std::move(that.s), std::move(that.r))
    {}

    explicit owning_ref_symbol(std::string str)
      : owning_ref_symbol(std::move(str), 0)
    {}
    explicit owning_ref_symbol(const char* str)
      : owning_ref_symbol(std::string{str})
    {}
    
    owning_ref_symbol& operator=(owning_ref_symbol that)
    {
        std::swap(s, that.s);
        std::swap(r, that.r);
        return *this;
    }
    
    symbol* refered();
    const symbol* refered() const;
    void refered(std::unique_ptr<any_symbol> reference)
    {
        r = std::move(reference);
    }
    const std::string& identifier() const
    {
        return s;
    }
    void identifier(std::string new_identifier)
    {
        s = std::move(new_identifier);
    }

    bool operator==(const owning_ref_symbol& that) const
    {
        return s == that.s && r == that.r;
    }
    bool operator!=(const owning_ref_symbol& that) const
    {
        return !(*this == that);
    }

private:
    std::string s;
    std::unique_ptr<any_symbol> r;
};
static_assert(std::is_nothrow_move_constructible<owning_ref_symbol>::value, "");

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
    
    const symbol* refered() const
    {
        return r;
    }
    void refered(const symbol* new_reference)
    {
        r = new_reference;
    }
    const std::string& identifier() const
    {
        return s;
    }
    void identifier(std::string new_identifier)
    {
        s = std::move(new_identifier);
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
  : public symbol_impl
{
public:
    typedef std::function<any_symbol (list_symbol::const_iterator begin, list_symbol::const_iterator end)> macro_function;
    macro_symbol(macro_function func)
      : f(std::move(func))
    {
        type_id = MACRO;
    }
    
    any_symbol operator()(list_symbol::const_iterator begin, list_symbol::const_iterator end) const;
private:
    macro_function f;
};

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
    assert(is_none());
    return *static_cast<none_symbol*>(this);
}
inline bool symbol::is_id() const
{
    return impl().type_id == ID;
}
inline id_symbol& symbol::cast_id()
{
    assert(is_id());
    return *static_cast<id_symbol*>(this);
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
inline bool symbol::is_owning_ref() const
{
    return impl().type_id == OWNING_REFERENCE;
}
inline owning_ref_symbol& symbol::owning_ref()
{
    assert(is_owning_ref());
    return *static_cast<owning_ref_symbol*>(this);
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
inline bool symbol::is_macro() const
{
    return impl().type_id == MACRO;
}
inline macro_symbol& symbol::macro()
{
    assert(is_macro());
    return *static_cast<macro_symbol*>(this);
}

inline bool operator==(const symbol& lhs, const symbol& rhs)
{
    if(lhs.type() != rhs.type())
        return false;
    
    switch(lhs.type())
    {
    case symbol::NONE:
        return lhs.cast_none() == rhs.cast_none();
    case symbol::ID:
        return lhs.cast_id() == rhs.cast_id();
    case symbol::LITERAL:
        return lhs.lit() == rhs.lit();
    case symbol::OWNING_REFERENCE:
        return lhs.owning_ref() == rhs.owning_ref();
    case symbol::REFERENCE:
        return lhs.ref() == rhs.ref();
    case symbol::LIST:
        return lhs.list() == rhs.list();
    case symbol::MACRO:
        return lhs.macro() == rhs.macro();
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
        sizeof(none_symbol), sizeof(lit_symbol), sizeof(owning_ref_symbol),
        sizeof(ref_symbol), sizeof(list_symbol));

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
        visit_none([](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type T;
            obj.~T();
        });
    }
};



inline symbol* owning_ref_symbol::refered()
{
    return r.get();
}
inline const symbol* owning_ref_symbol::refered() const
{
    return r.get();
}

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

