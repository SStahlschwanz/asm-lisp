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
    BRANCH,
    PHI,
    CMP,
    RETURN,
    CALL,

    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,

    LET,

    FIRST_UNUSED
};

}

#endif

