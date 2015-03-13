#ifndef SAVE_HPP_
#define SAVE_HPP_

#include "utils.hpp"

#include <vector>
#include <string>

namespace save_impl
{

template<class Container>
struct save;

template<class T>
struct save<std::vector<T>>
{
    template<class Range>
    static std::vector<T> doit(const Range& r)
    {
        std::vector<T> result;
        for_each(r, [&](auto obj)
        {
            result.push_back(std::move(obj)); // TODO: does this work for all types of references?
        });
        return result;
    }
};
template<>
struct save<std::string>
{
    template<class Range>
    static std::string doit(const Range& r)
    {
        std::string result;
        for_each(r, [&](auto obj)
        {
            result.push_back(std::move(obj)); // TODO: does this work for all types of references?
        });
        return result;
    }
};

}

template<class Container, class Range>
Container save(const Range& r)
{
    return save_impl::save<Container>::doit(r);
}

#endif

