#ifndef CORE_UNIQUE_IDS_HPP_
#define CORE_UNIQUE_IDS_HPP_

#include <cstddef>

namespace unique_ids
{

enum 
  : std::size_t
{
    ADD = 1,
    SUB,
    MUL,
    DIV,
    ALLOC,
    STORE,
    LOAD,
    COND_BRANCH,
    CMP,
    RETURN,
    CALL,

    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,

    FIRST_UNUSED
};

}

#endif

