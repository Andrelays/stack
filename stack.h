#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

extern FILE *Global_logs_pointer;
extern bool  Global_color_output;

#define STACK_CONSTRUCTOR(stk)                                                          \
do {                                                                                    \
    struct debug_info *info = (debug_info *) calloc(1, sizeof(debug_info));             \
                                                                                        \
    info->line = __LINE__;                                                              \
    info->name = #stk;                                                                  \
    info->file = __FILE__;                                                              \
    info->func = __PRETTY_FUNCTION__;                                                   \
                                                                                        \
    stack_constructor(stk, info);                                                       \
} while(0)

#ifdef CANARY_PROTECT_INCLUDED

    typedef long long canary_t;

    #define IF_ON_CANARY_PROTECT(...)           __VA_ARGS__
    #define ELSE_IF_OFF_CANARY_PROTECT(...)


#else

    #define IF_ON_CANARY_PROTECT(...)
    #define ELSE_IF_OFF_CANARY_PROTECT(...)     __VA_ARGS__

#endif

#ifdef HASH_PROTECT_INCLUDED

    #define IF_ON_HASH_PROTECT(...)             __VA_ARGS__

#else

    #define IF_ON_HASH_PROTECT(...)

#endif

#define FORMAT_SPECIFIERS_STACK   "%d"
typedef int TYPE_ELEMENT_STACK;

const ssize_t  CAPACITY_MULTIPLIER      = 2;
const ssize_t  INITIAL_CAPACITY_VALUE   = 1;
const int      POISON                   = 192;

enum errors_code_stack {
    NO_ERROR                        = 0,
    POINTER_TO_STACK_IS_NULL        = 1,
    POINTER_TO_STACK_DATA_IS_NULL   = 1 <<  1,
    SIZE_MORE_THAN_CAPACITY         = 1 <<  2,
    CAPACITY_LESS_THAN_ZERO         = 1 <<  3,
    SIZE_LESS_THAN_ZERO             = 1 <<  4,
    SIZE_NULL_IN_POP                = 1 <<  5,
    POINTER_TO_STACK_INFO_IS_NULL   = 1 <<  6,
    POINTER_RETURN_VALUE_POP_NULL   = 1 <<  7,

    IF_ON_CANARY_PROTECT(
    LEFT_CANARY_IN_STACK_CHANGED    = 1 <<  8,
    RIGHT_CANARY_IN_STACK_CHANGED   = 1 <<  9,
    LEFT_CANARY_IN_ARRAY_CHANGED    = 1 << 10,
    RIGHT_CANARY_IN_ARRAY_CHANGED   = 1 << 11,
    )

    IF_ON_HASH_PROTECT(
    STACK_HASH_CHANGED              = 1 << 12,
    DATA_HASH_CHANGED               = 1 << 13
    )
};

struct stack {
    TYPE_ELEMENT_STACK             *data;
    ssize_t                         size;
    ssize_t                         capacity;
    ssize_t                         error_code;
    struct debug_info              *info;

    IF_ON_CANARY_PROTECT (canary_t left_canary;)
    IF_ON_CANARY_PROTECT (canary_t right_canary;)

    IF_ON_HASH_PROTECT(uint32_t stack_hash);
    IF_ON_HASH_PROTECT(uint32_t data_hash);
};

struct debug_info {
    ssize_t      line;
    const char  *name;
    const char  *file;
    const char  *func;
};

stack *get_pointer_stack();

ssize_t stack_constructor(stack *stk, debug_info *info);
ssize_t stack_destructor(stack *stk);

ssize_t push(stack *stk, TYPE_ELEMENT_STACK value);
ssize_t pop(stack *stk, TYPE_ELEMENT_STACK *return_value);

#endif  //STACK_H_INCLUDED
