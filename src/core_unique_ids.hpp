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
    SDIV,
    ALLOC,
    STORE,
    LOAD,
    COND_BRANCH,
    BRANCH,
    PHI,
    CMP,
    RETURN,
    CALL,

    IS_ID,
    IS_LIT,
    IS_REF,
    IS_LIST,
    IS_MACRO,

    LIT_CREATE,
    LIT_SIZE,
    LIT_PUSH,
    LIT_POP,
    LIT_GET,
    LIT_SET,
    
    LIST_CREATE,
    LIST_SIZE,
    LIST_PUSH,
    LIST_POP,
    LIST_GET,
    LIST_SET,

    REF_CREATE,
    REF_GET_IDENTIFIER,
    REF_SET_IDENTIFIER,
    REF_HAS_REFERED,
    REF_GET_REFERED,
    REF_SET_REFERED,

    TO_NODE,
    CALL_MACRO,
    
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
    NODE,
    VOID,
    
    // key words
    LET,

    FIRST_UNUSED
};

}

#endif

