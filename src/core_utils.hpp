#ifndef CORE_UTILS_HPP_
#define CORE_UTILS_HPP_

#include "node.hpp"
#include "error/core_misc_error.hpp"

// TODO: only works for non-cyclic graphs
inline const node& resolve_refs(const node& n)
{
    using namespace core_misc_error;

    if(n.is<ref_node>())
    {
        const ref_node& r = n.cast<ref_node>();
        if(r.refered() == 0)
            core_misc_error::fatal<core_misc_error::id("identifier_not_defined")>(n.source());
        else
            return resolve_refs(*r.refered());
    }
    else
        return n;
}

#endif

