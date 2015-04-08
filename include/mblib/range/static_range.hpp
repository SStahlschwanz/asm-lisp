#ifndef STATIC_RANGE_HPP_
#define STATIC_RANGE_HPP_

#include <mblib/range/traits.hpp>
#include <cstddef>

template<class T>
class single_object_range
{
private:
    T* obj_;
public:
    typedef range_tag range_tag;

    single_object_range(T& obj)
      : obj_(&obj)
    {}

    bool empty() const
    {
        return obj_ == nullptr;
    }
    std::size_t length() const
    {
        return static_cast<std::size_t>(obj_ != nullptr);
    }

    T& front() const
    {
        assert(obj_);
        return *obj_;
    }
    void pop_front()
    {
        assert(obj_);
        obj_ = nullptr;
    }
};

template<class T>
single_object_range<T> static_range(T& obj)
{
    return {obj};
}


#endif

