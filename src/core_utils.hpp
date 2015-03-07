#ifndef CORE_UTILS_HPP_
#define CORE_UTILS_HPP_

#include "symbol.hpp"
#include "error/core_misc_error.hpp"

inline const symbol& resolve_refs(const symbol& s)
{
    using namespace core_misc_error;

    if(s.is<ref_symbol>())
    {
        const ref_symbol& r = s.cast<ref_symbol>();
        if(r.refered() == 0)
            core_misc_error::fatal<core_misc_error::id("identifier_not_defined")>(s.source());
        else
            return resolve_refs(*r.refered());
    }
    else
        return s;
}

#endif

