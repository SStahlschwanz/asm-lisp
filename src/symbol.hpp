#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include <boost/variant.hpp>

#include <vector>
#include <initializer_list>
#include <memory>

#include "symbol_source.hpp"
#include "compilation_context.hpp"

template<class>
class symbol_with_data;
template<class>
class id_symbol_with_data;
template<class>
class lit_symbol_with_data;
template<class>
class ref_symbol_with_data;
template<class>
class list_symbol_with_data;
template<class>
class macro_symbol_with_data;
template<class>
class any_symbol_with_data;

namespace symbol_detail
{

template<class>
class symbol_impl_with_data;

}

struct empty_symbol_data
{};
typedef symbol_with_data<empty_symbol_data> symbol;
typedef lit_symbol_with_data<empty_symbol_data> lit_symbol;
typedef ref_symbol_with_data<empty_symbol_data> ref_symbol;
typedef list_symbol_with_data<empty_symbol_data> list_symbol;
typedef id_symbol_with_data<empty_symbol_data> id_symbol;
typedef macro_symbol_with_data<empty_symbol_data> macro_symbol;
typedef any_symbol_with_data<empty_symbol_data> any_symbol;

namespace symbol_shortcuts
{

typedef lit_symbol lit;
typedef ref_symbol ref;
typedef list_symbol list;
typedef macro_symbol macro;

}


enum class symbol_type_id
{
    ID,
    LITERAL,
    REFERENCE,
    LIST,
    MACRO
};

template<class Data>
class symbol_with_data
{
public:
    symbol_type_id type() const;
    const symbol_source& source() const;
    void source(const symbol_source& new_source);
    
    template<class SymbolType>
    bool is() const;
    template<class SymbolType>
    SymbolType& cast();
    template<class SymbolType>
    const SymbolType& cast() const
    {
        return const_cast<symbol_with_data*>(this)->cast<SymbolType>();
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
        return const_cast<symbol_with_data*>(this)->cast_else<SymbolType>(
                std::forward<FunctorType>(functor));
    }

    template<class FunctorType>
    void visit(FunctorType&& functor);
    template<class FunctorType>
    void visit(FunctorType&& functor) const
    {
        const_cast<symbol_with_data*>(this)->visit([&](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type actual_type;
            functor(const_cast<const actual_type&>(obj));
        });
    }

    Data& data();
    const Data& data() const;
private:
    template<class>
    friend class any_symbol_with_data;
    template<class>
    friend class symbol_detail::symbol_impl_with_data;
    ~symbol_with_data()
    {}
    
    symbol_detail::symbol_impl_with_data<Data>& impl();
    const symbol_detail::symbol_impl_with_data<Data>& impl() const;
};

namespace symbol_detail
{
template<class Data>
class symbol_impl_with_data
  : public symbol_with_data<Data>
{
    static_assert(std::is_nothrow_constructible<Data>::value, "");
private:
    friend class ::symbol_with_data<Data>;
    friend class ::id_symbol_with_data<Data>;
    friend class ::lit_symbol_with_data<Data>;
    friend class ::ref_symbol_with_data<Data>;
    friend class ::list_symbol_with_data<Data>;
    friend class ::macro_symbol_with_data<Data>;

    symbol_impl_with_data(symbol_type_id type_val)
      : t(type_val)
    {}
    ~symbol_impl_with_data() = default;
    symbol_type_id t;
    symbol_source source_value;
    Data d;
};
}

template<class Data>
class id_symbol_with_data
  : public symbol_detail::symbol_impl_with_data<Data>
{
public:
    static constexpr symbol_type_id type_id = symbol_type_id::ID;

    id_symbol_with_data(size_t new_id)
      : symbol_detail::symbol_impl_with_data<Data>(type_id),
        i(new_id)
    {}
    id_symbol_with_data()
      : id_symbol_with_data(0)
    {}
    id_symbol_with_data(const id_symbol_with_data&) = default;
    id_symbol_with_data(id_symbol_with_data&& that) noexcept
      : id_symbol_with_data{that.i}
    {}
    bool operator==(const id_symbol_with_data& that) const
    {
        return i == that.i;
    }
    bool operator!=(const id_symbol_with_data& that) const
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
static_assert(std::is_nothrow_move_constructible<id_symbol>::value, "");

template<class Data>
class lit_symbol_with_data
  : public symbol_detail::symbol_impl_with_data<Data>
{
public:
    static constexpr symbol_type_id type_id = symbol_type_id::LITERAL;

    lit_symbol_with_data(std::string str)
      : symbol_detail::symbol_impl_with_data<Data>(type_id),
        s(std::move(str))
    {}
    lit_symbol_with_data(const char* str)
      : lit_symbol_with_data(std::string{str})
    {}
    lit_symbol_with_data()
      : lit_symbol_with_data("")
    {}
    
    lit_symbol_with_data(lit_symbol_with_data&& that) noexcept
      : symbol_detail::symbol_impl_with_data<Data>(std::move(that)),
        s(std::move(that.s))
    {}
    lit_symbol_with_data(const lit_symbol_with_data&) = default;

    lit_symbol_with_data& operator=(lit_symbol_with_data that)
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

    bool operator==(const lit_symbol_with_data& that) const
    {
        return s == that.s;
    }
    bool operator!=(const lit_symbol_with_data& that) const
    {
        return !(*this == that);
    }

private:
    std::string s;
};
static_assert(std::is_nothrow_move_constructible<lit_symbol>::value, "");

template<class Data>
class ref_symbol_with_data
  : public symbol_detail::symbol_impl_with_data<Data>
{
public:
    static constexpr symbol_type_id type_id = symbol_type_id::REFERENCE;
    ref_symbol_with_data(identifier_id_t identifier_id, const symbol_with_data<Data>* reference)
      : symbol_detail::symbol_impl_with_data<Data>(type_id),
        identifier_id(identifier_id),
        r(reference)
    {}
#ifndef NDEBUG
    ref_symbol_with_data(identifier_id_t identifier_id, std::string str, const symbol_with_data<Data>* reference)
      : symbol_detail::symbol_impl_with_data<Data>(type_id),
        s(std::move(str)),
        identifier_id(identifier_id),
        r(reference)
    {}
#endif

    ref_symbol_with_data(const ref_symbol_with_data&) = default;
    ref_symbol_with_data(ref_symbol_with_data&& that) noexcept
      : symbol_detail::symbol_impl_with_data<Data>(std::move(that)),
        identifier_id(that.identifier_id),
        r(that.r)
    {}

    explicit ref_symbol_with_data(identifier_id_t identifier_id)
      : ref_symbol_with_data(identifier_id, nullptr)
    {}
    ref_symbol_with_data& operator=(ref_symbol_with_data that)
    {
        std::swap(identifier_id, that.identifier_id);
        std::swap(r, that.r);
        return *this;
    }
    
    const symbol_with_data<Data>* refered() const
    {
        return r;
    }
    void refered(const symbol_with_data<Data>* new_reference)
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

    bool operator==(const ref_symbol_with_data& that) const
    {
        return identifier_id == that.identifier_id && r == that.r;
    }
    bool operator!=(const ref_symbol_with_data& that) const
    {
        return !(*this == that);
    }

private:
#ifndef NDEBUG
    std::string s;
#endif
    identifier_id_t identifier_id;
    const symbol_with_data<Data>* r;
};
static_assert(std::is_nothrow_move_constructible<ref_symbol>::value, "");

template<class Data> 
class list_symbol_with_data
  : public symbol_detail::symbol_impl_with_data<Data>
{
public:
    static constexpr symbol_type_id type_id = symbol_type_id::LIST;
    list_symbol_with_data(std::vector<any_symbol_with_data<Data>> vec)
      : symbol_detail::symbol_impl_with_data<Data>(type_id),
        v(std::move(vec))
    {}
    list_symbol_with_data()
      : list_symbol_with_data(std::vector<any_symbol_with_data<Data>>{})
    {}
    list_symbol_with_data(std::initializer_list<any_symbol_with_data<Data>> l);
    
    list_symbol_with_data(const list_symbol_with_data&) = default;
    list_symbol_with_data(list_symbol_with_data&& that) noexcept
      : symbol_detail::symbol_impl_with_data<Data>(std::move(that)),
        v(std::move(that.v))
    {}

    list_symbol_with_data& operator=(list_symbol_with_data that)
    {
        std::swap(v, that.v);
        return *this;
    }

    typedef typename std::vector<any_symbol_with_data<Data>>::iterator iterator;
    typedef typename std::vector<any_symbol_with_data<Data>>::const_iterator const_iterator;
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
    any_symbol_with_data<Data>& operator[](size_t i);
    const any_symbol_with_data<Data>& operator[](size_t i) const;
    void push_back(symbol_with_data<Data>&& s);
    void push_back(const symbol_with_data<Data>& s);
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

    bool operator==(const list_symbol_with_data<Data>& that) const
    {
        return v == that.v;
    }
    bool operator!=(const list_symbol_with_data<Data>& that) const
    {
        return !(*this == that);
    }

private:
    std::vector<any_symbol_with_data<Data>> v;
};
static_assert(std::is_nothrow_move_constructible<list_symbol>::value, "");

template<class Data>
class macro_symbol_with_data
  : public symbol_detail::symbol_impl_with_data<Data>
{
public:
    static constexpr symbol_type_id type_id = symbol_type_id::LIST;
    
    typedef std::function<any_symbol (list_symbol::const_iterator, list_symbol::const_iterator)> macro_function;

    macro_symbol_with_data(macro_function function)
      : symbol_detail::symbol_impl_with_data<Data>(type_id),
        func(std::move(function))
    {}
    macro_symbol_with_data(const macro_symbol_with_data&) = default;
    macro_symbol_with_data(macro_symbol&& that) noexcept
      : macro_symbol{std::move(that.func)}
    {}

    any_symbol operator()(list_symbol::const_iterator begin, list_symbol::const_iterator end) const;
    bool operator==(const macro_symbol_with_data& that) const
    {
        // TODO
        assert(false);
    }
    bool operator!=(const macro_symbol_with_data& that) const
    {
        return !(*this == that);
    }

private:
    macro_function func;
};
static_assert(std::is_nothrow_move_constructible<macro_symbol>::value, "");

template<class Data>
symbol_detail::symbol_impl_with_data<Data>& symbol_with_data<Data>::impl()
{
    return *static_cast<symbol_detail::symbol_impl_with_data<Data>*>(this);
}
template<class Data>
const symbol_detail::symbol_impl_with_data<Data>& symbol_with_data<Data>::impl() const
{
    return *static_cast<const symbol_detail::symbol_impl_with_data<Data>*>(this);
}
template<class Data>
symbol_type_id symbol_with_data<Data>::type() const
{
    return impl().t;
}
template<class Data>
const symbol_source& symbol_with_data<Data>::source() const
{
    return impl().source_value;
}
template<class Data>
void symbol_with_data<Data>::source(const symbol_source& new_source)
{
    impl().source_value = new_source;
}
template<class Data> template<class SymbolType>
bool symbol_with_data<Data>::is() const
{
    return type() == SymbolType::type_id;
}
template<class Data> template<class SymbolType>
SymbolType& symbol_with_data<Data>::cast()
{
    assert(is<SymbolType>());
    return *static_cast<SymbolType*>(this);
}
template<class Data> template<class FunctorType>
void symbol_with_data<Data>::visit(FunctorType&& f)
{
    switch(type())
    {
    case symbol_type_id::ID:
        f(cast<id_symbol_with_data<Data>>());
        break;
    case symbol_type_id::LITERAL:
        f(cast<lit_symbol_with_data<Data>>());
        break;
    case symbol_type_id::REFERENCE:
        f(cast<ref_symbol_with_data<Data>>());
        break;
    case symbol_type_id::LIST:
        f(cast<list_symbol_with_data<Data>>());
        break;
    case symbol_type_id::MACRO:
        f(cast<macro_symbol_with_data<Data>>());
        break;
    }
}

template<class Data>
Data& symbol_with_data<Data>::data()
{
    return impl().d;
}
template<class Data>
const Data& symbol_with_data<Data>::data() const
{
    return impl().d;
}

template<class Data>
inline bool operator==(const symbol_with_data<Data>& lhs, const symbol_with_data<Data>& rhs)
{
    if(lhs.type() != rhs.type())
        return false;
    
    switch(lhs.type())
    {
    case symbol_type_id::ID:
        return lhs.template cast<id_symbol_with_data<Data>>() == rhs.template cast<id_symbol_with_data<Data>>();
    case symbol_type_id::LITERAL:
        return lhs.template cast<lit_symbol_with_data<Data>>() == rhs.template cast<lit_symbol_with_data<Data>>();
    case symbol_type_id::REFERENCE:
        return lhs.template cast<ref_symbol_with_data<Data>>() == rhs.template cast<ref_symbol_with_data<Data>>();
    case symbol_type_id::LIST:
        return lhs.template cast<list_symbol_with_data<Data>>() == rhs.template cast<list_symbol_with_data<Data>>();
    case symbol_type_id::MACRO:
        return lhs.template cast<macro_symbol_with_data<Data>>() == rhs.template cast<macro_symbol_with_data<Data>>();
    }
}
template<class Data>
inline bool operator!=(const symbol_with_data<Data>& lhs, const symbol_with_data<Data>& rhs)
{
    return !(lhs == rhs);
}

namespace symbol_detail
{

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

}

template<class Data>
class any_symbol_with_data
  : public symbol_with_data<Data>
{
    constexpr static const size_t max_symbol_size = symbol_detail::constexpr_max_for_symbol(
        sizeof(lit_symbol_with_data<Data>), sizeof(ref_symbol_with_data<Data>),
        sizeof(list_symbol_with_data<Data>), sizeof(macro_symbol_with_data<Data>),
        sizeof(id_symbol_with_data<Data>));
public:
    any_symbol_with_data(const symbol_with_data<Data>& that)
    {
        that.visit([&](auto obj)
        {
            construct(obj);
        });
    }
    any_symbol_with_data(symbol_with_data<Data>&& that) noexcept
    {
        that.visit([&](auto& obj)
        {
            construct(std::move(obj));
        });
    }
    any_symbol_with_data(const any_symbol_with_data<Data>& that)
      : any_symbol_with_data(static_cast<const symbol_with_data<Data>&>(that))
    {}
    any_symbol_with_data(any_symbol_with_data&& that)
      : any_symbol_with_data(static_cast<symbol_with_data<Data>&&>(that))
    {}
    
    ~any_symbol_with_data() noexcept
    {
        destruct();
    }
    
    any_symbol_with_data& operator=(symbol_with_data<Data>&& that) noexcept
    {
        destruct();
        that.visit([&](auto& obj)
        {
            construct(std::move(obj));
        });
        return *this;
    }
    any_symbol_with_data& operator=(const symbol_with_data<Data>& that)
    {
        that.visit([&](auto obj)
        {
            // obj has been copied already
            destruct(); // noexcept
            construct(std::move(obj)); // noexcept
        });
        return *this;
    }
    any_symbol_with_data& operator=(any_symbol_with_data<Data> that)
    {
        return (*this = static_cast<symbol_with_data<Data>&&>(that));
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
        this->visit([](auto& obj)
        {
            typedef typename std::decay<decltype(obj)>::type T;
            obj.~T();
        });
    }
};


template<class Data>
list_symbol_with_data<Data>::list_symbol_with_data(std::initializer_list<any_symbol_with_data<Data>> l)
  : list_symbol_with_data(std::vector<any_symbol_with_data<Data>>{l.begin(), l.end()})
{}
template<class Data>
any_symbol_with_data<Data>& list_symbol_with_data<Data>::operator[](size_t i)
{
    return v[i];
}
template<class Data>
const any_symbol_with_data<Data>& list_symbol_with_data<Data>::operator[](size_t i) const
{
    return v[i];
}
template<class Data>
void list_symbol_with_data<Data>::push_back(symbol_with_data<Data>&& s)
{
    v.push_back(std::move(s));
}
template<class Data>
void list_symbol_with_data<Data>::push_back(const symbol_with_data<Data>& s)
{
    v.push_back(s);
}

template<class Data>
any_symbol macro_symbol_with_data<Data>::operator()(list_symbol::const_iterator begin, list_symbol::const_iterator end) const
{
    return func(begin, end);
}

#endif

