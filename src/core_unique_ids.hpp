#ifndef CORE_UNIQUE_IDS_HPP_
#define CORE_UNIQUE_IDS_HPP_

#include <cstddef>

namespace unique_ids
{

enum 
  : std::size_t
{
    // instruction constructors
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
    
    // cmp kinds
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    
    // type constructors
    INT,
    
    // key words
    LET,

    FIRST_UNUSED
};

}

#endif

