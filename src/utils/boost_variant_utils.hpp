#ifndef BOOST_VARIANT_UTILS_HPP_
#define BOOST_VARIANT_UTILS_HPP_

#include "functor_utils.hpp"

#include <boost/variant.hpp>

#include <boost/mpl/vector.hpp>

namespace boost_variant_utils_detail
{


template<class MPLVector, class... Types>
struct variadic_make_variant;
template<class MPLVector>
struct variadic_make_variant<MPLVector>
{
    typedef typename boost::make_variant_over<MPLVector>::type type;
};
template<class MPLVector, class Head, class... Tail>
struct variadic_make_variant<MPLVector, Head, Tail...>
{
    typedef typename boost::mpl::push_front<MPLVector, Head>::type next_vector;
    typedef typename variadic_make_variant<next_vector, Tail...>::type type;
};

}

template<class... Types>
struct variadic_make_variant
{
    typedef typename boost_variant_utils_detail::variadic_make_variant<boost::mpl::vector<>, Types...>::type type;

};

template<class ReturnType = void, class VariantType, class... Visitors>
ReturnType visit(VariantType& variant, Visitors... visitors)
{
    return boost::apply_visitor(overloaded<ReturnType>(std::move(visitors)...), variant);
}


#endif

