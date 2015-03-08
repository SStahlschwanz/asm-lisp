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

    LIST_CREATE,
    LIST_SIZE,
    LIST_PUSH,
    LIST_POP,
    LIST_GET,
    LIST_SET,
    
    // cmp kinds
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    
    // type constructors
    INT,
    PTR,
    
    // key words
    LET,

    FIRST_UNUSED
};

}

#endif

