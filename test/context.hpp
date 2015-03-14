#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include "../src/compilation_context.hpp"

inline compilation_context& context()
{
    static compilation_context c;
    return c;
}

#endif

