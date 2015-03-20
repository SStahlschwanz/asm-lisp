#ifndef NODE_HPP_
#define NODE_HPP_

#include "node_source.hpp"
#include "dynamic_graph.hpp"

#include <mblib/range.hpp>
#include <mblib/functor.hpp>

#include <cstddef>
#include <cassert>
#include <vector>
#include <initializer_list>
#include <memory>
#include <iterator>
#include <functional>

enum class node_type
{
    ID,
    LITERAL,
    REFERENCE,
    LIST,
    MACRO,
    PROC
};

class node
{
public:
    node_type type() const
    {
        return nt_;
    }
    const node_source& source() const
    {
        return source_;
    }
    void source(const node_source& source)
    {
        source_ = source;
    }
    template<class NodeType>
    bool is() const
    {
        return type() == NodeType::type_id;
    }
    template<class NodeType>
    NodeType& cast()
    {
        assert(is<NodeType>());
        return *static_cast<NodeType*>(this);
    }
    template<class NodeType>
    const NodeType& cast() const
    {
        return const_cast<node*>(this)->cast<NodeType>();
    }
    template<class NodeType, class FunctorType>
    NodeType& cast_else(FunctorType&& functor)
    {
        if(is<NodeType>())
            return cast<NodeType>();
        else
        {
            functor();
            assert(false);
            return *static_cast<NodeType*>(nullptr); // to suppress warnings
        }

    }
    template<class NodeType, class FunctorType>
    const NodeType& cast_else(FunctorType&& functor) const
    {
        return const_cast<node*>(this)->cast_else<NodeType>(functor);
    }
    template<class ReturnType = void, class FunctorType>
    ReturnType visit(FunctorType&& functor);
    template<class ReturnType = void, class FunctorType>
    ReturnType visit(FunctorType&& functor) const
    {
        return const_cast<node*>(this)->visit<ReturnType>([&](auto& obj) -> ReturnType
        {
            typedef std::decay_t<decltype(obj)> actual_type;
            return functor(const_cast<const actual_type&>(obj));
        });
    }
    template<class ResultType = void, class Functor1Type, class Functor2Type, class... FunctorTypes>
    ResultType visit(Functor1Type&& functor1, Functor2Type&& functor2, FunctorTypes&&... functors)
    {
        return visit<ResultType>(overloaded<ResultType>(std::forward<Functor1Type>(functor1), std::forward<Functor2Type>(functor2), std::forward<FunctorTypes>(functors)...));
    }
    template<class ResultType = void, class Functor1Type, class Functor2Type, class... FunctorTypes>
    ResultType visit(Functor1Type&& functor1, Functor2Type&& functor2, FunctorTypes&&... functors) const
    {
        return visit<ResultType>(overloaded<ResultType>(std::forward<Functor1Type>(functor1), std::forward<Functor2Type>(functor2), std::forward<FunctorTypes>(functors)...));
    }
private:
    friend class id_node;
    friend class lit_node;
    friend class ref_node;
    friend class list_node;
    friend class macro_node;
    friend class proc_node;

    node(node_type nt)
      : nt_(nt)
    {}

    node_type nt_;
    node_source source_;
};

class id_node
  : public node
{
public:
    static constexpr node_type type_id = node_type::ID;

    id_node(std::size_t new_id)
      : node(type_id),
        id_(new_id)
    {}
    
    std::size_t id() const
    {
        return id_;
    }
    void id(std::size_t new_id)
    {
        id_ = new_id;
    }
private:
    std::size_t id_;
};

class lit_node
  : public node
{
public:
    static constexpr node_type type_id = node_type::LITERAL;

    lit_node(char* begin, char* end)
      : node(type_id),
        begin_(begin),
        end_(end)
    {}
    
    typedef char* iterator;
    char* begin() const // todo const correctness
    {
        return begin_;
    }
    char* end() const
    {
        return end_;
    }
private:
    char* begin_;
    char* end_;
};

class ref_node
  : public node
{
public:
    static constexpr node_type type_id = node_type::REFERENCE;
    
    ref_node(char* begin, char* end, node* new_refered)
      : node(type_id),
        begin_(begin),
        end_(end),
        refered_(new_refered)
    {}
    
    auto identifier() const // todo: const correctness
    {
        return rangeify(begin_, end_);
    }
    const node* refered() const
    {
        return refered_;
    }
    void refered(const node* new_refered)
    {
        refered_ = new_refered;
    }
private:
    char* begin_;
    char* end_;
    const node* refered_;
};

class list_node
  : public node
{
public:
    static constexpr node_type type_id = node_type::LIST;

    list_node(node** begin, node** end)
      : node(type_id),
        begin_(begin),
        end_(end)
    {}

    struct iterator
    {
        node** pos;

        iterator& operator++()
        {
            ++pos;
            return *this;
        }
        iterator operator+(std::size_t distance)
        {
            return iterator{pos + distance};
        }
        
        node& operator*() const
        {
            return **pos;
        }
        node* operator->() const
        {
            return *pos;
        }

        bool operator==(const iterator& that) const
        {
            return pos == that.pos;
        }
        bool operator!=(const iterator& that) const
        {
            return !(*this == that);
        }
    };
    iterator begin() const
    {
        return {begin_};
    }
    iterator end() const
    {
        return {end_};
    }
    
    bool empty() const
    {
        return begin_ == end_;
    }
    std::size_t size() const
    {
        return std::distance(begin_, end_);
    }
    node& operator[](std::size_t index)
    {
        assert(index < size());
        return **(begin_ + index);
    }
    const node& operator[](std::size_t index) const
    {
        return const_cast<list_node&>(*this)[index];
    }
private:
    node** begin_;
    node** end_;
};

typedef decltype(rangeify(std::declval<const list_node>())) node_range;

class macro_node
  : public node
{
public:
    static constexpr node_type type_id = node_type::MACRO;

    typedef std::pair<node&, dynamic_graph> macro(node_range);

    std::pair<node&, dynamic_graph> operator()(decltype(rangeify(std::declval<const list_node>())) r) const
    {
        return (*func_)(std::move(r));
    }
    macro_node(std::shared_ptr<std::function<macro>> function)
      : node(type_id),
        func_(std::move(function))
    {}

    const std::shared_ptr<std::function<macro>>& function()
    {
        return func_;
    }
    void function(std::shared_ptr<std::function<macro>> new_func)
    {
        func_ = std::move(new_func);
    }
private:
    std::shared_ptr<std::function<macro>> func_;
};

namespace llvm
{

class Function;

}

class proc_node
  : public node
{
public:
    static constexpr node_type type_id = node_type::PROC;

    proc_node(llvm::Function* ct_func, llvm::Function* rt_func)
      : node(type_id),
        ct_func_(ct_func),
        rt_func_(rt_func)
    {}
    llvm::Function* ct_function() const
    {
        return ct_func_;
    }
    void ct_function(llvm::Function* func)
    {
        ct_func_ = func;
    }
    llvm::Function* rt_function() const
    {
        return rt_func_;
    }
    void rt_function(llvm::Function* func)
    {
        rt_func_ = func;
    }
private:
    llvm::Function* ct_func_;
    llvm::Function* rt_func_;
};

template<class ReturnType, class FunctorType>
ReturnType node::visit(FunctorType&& functor)
{
    switch(type())
    {
    case node_type::ID:
        return functor(this->template cast<id_node>());
    case node_type::LITERAL:
        return functor(this->template cast<lit_node>());
    case node_type::REFERENCE:
        return functor(this->template cast<ref_node>());
    case node_type::LIST:
        return functor(this->template cast<list_node>());
    case node_type::MACRO:
        return functor(this->template cast<macro_node>());
    case node_type::PROC:
        return functor(this->template cast<proc_node>());
    }
}

/*
namespace node_detail
{

bool structurally_equal(const node& lhs, const node& rhs, DereferenceFunctor&& dereference, IsNullFunctor&& is_null)
{
    if(lhs.type() != rhs.type())
        return false;
    switch(lhs.type())
    {
    case node_type::ID:
        return lhs.cast<id_node>().id() == rhs.cast<id_node>().id();
    case node_type::LITERAL:
        return rangeify(lhs.cast<lit_node>()) == rangeify(rhs.cast<lit_node>());
    case node_type::REFERENCE:
    {
        const ref_node& lhs_ref = lhs.cast<ref_node>();
        const ref_node& rhs_ref = rhs.cast<ref_node>();
        if(lhs_ref.identifier() != rhs_ref.identifier())
            return false;
        if(is_null(lhs_ref.refered())) // rhs_ref.refered() is also null
            return true;

        return structurally_equal(dereference(lhs_refered.refered()), dereference(rhs_refered.refered()), dereference, is_null);
    }
    case node_type::LIST:
    {
        auto lhs_range = rangeify(lhs.cast<list_node>());
        auto rhs_range = rangeify(rhs.cast<list_node>());
        
        if(length(lhs_range) != length(rhs_range))
            return false;
        
        bool is_equal = true;
        for_each(zipped(lhs_range, rhs_range), unpacking([&](const node& lhs_node, const node& rhs_node)
        {
            is_equal = is_equal && structrually_equal(dereference(&lhs_node), dereference(&rhs_node));
        }));
        return is_equal;
    }        
    case node_type::MACRO:
        return true; // TODO
    case node_type::PROC:
        return true; // TODO
    }
}

}
*/

// TODO: only works for trees
inline bool structurally_equal(const node& lhs, const node& rhs)
{
    if(lhs.type() != rhs.type())
        return false;
    switch(lhs.type())
    {
    case node_type::ID:
        return lhs.cast<id_node>().id() == rhs.cast<id_node>().id();
    case node_type::LITERAL:
        return rangeify(lhs.cast<lit_node>()) == rangeify(rhs.cast<lit_node>());
    case node_type::REFERENCE:
    {
        const ref_node& lhs_ref = lhs.cast<ref_node>();
        const ref_node& rhs_ref = rhs.cast<ref_node>();
        if(lhs_ref.identifier() != rhs_ref.identifier())
            return false;
        if(lhs_ref.refered() == nullptr || rhs_ref.refered() == nullptr)
            return lhs_ref.refered() == rhs_ref.refered();

        return structurally_equal(*lhs_ref.refered(), *rhs_ref.refered());
    }
    case node_type::LIST:
    {
        auto lhs_range = rangeify(lhs.cast<list_node>());
        auto rhs_range = rangeify(rhs.cast<list_node>());
        
        if(length(lhs_range) != length(rhs_range))
            return false;
        
        bool is_equal = true;
        for_each(zipped(lhs_range, rhs_range), unpacking([&](const node& lhs_node, const node& rhs_node)
        {
            is_equal = is_equal && structurally_equal(lhs_node, rhs_node);
        }));
        return is_equal;
    }        
    case node_type::MACRO:
        return true; // TODO
    case node_type::PROC:
        return true; // TODO
    }
}

#endif

