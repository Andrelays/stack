#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#define STACK_CONSTRUCTOR(stk)                                                          \
do {                                                                                    \
    struct debug_info *info = (debug_info *) calloc(1, sizeof(debug_info));             \
                                                                                        \
    info->line = __LINE__;                                                              \
    info->name = #stk;                                                                  \
    info->file = __FILE__;                                                              \
    info->func = __PRETTY_FUNCTION__;                                                   \
                                                                                        \
    (*stk).info = info;                                                                 \
    stack_constructor(stk);                                                             \
} while(0)

#define STACK_DUMP(stk)                                                 \
do {                                                                    \
    stack_dump(stk, __LINE__, __FILE__, __PRETTY_FUNCTION__);           \
} while(0)

#define CHECK_ERRORS(stk)                                       \
do {                                                            \
    if (((stk)->error_code = verify_stack(stk)) != NO_ERROR)    \
        return (stk)->error_code;                               \
} while(0)

#define FORMAT_SPECIFIERS_STACK   "%d"

typedef int TYPE_ELEMENT_STACK;
typedef long long canary_t;

const canary_t VALUE_LEFT_CANARY_STACK  = 100;
const canary_t VALUE_RIGHT_CANARY_STACK = 200;
const canary_t VALUE_LEFT_CANARY_ARRAY  = 700;
const canary_t VALUE_RIGHT_CANARY_ARRAY = 900;
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
    LEFT_CANARY_IN_STACK_CHANGED    = 1 <<  6,
    RIGHT_CANARY_IN_STACK_CHANGED   = 1 <<  7,
    LEFT_CANARY_IN_ARRAY_CHANGED    = 1 <<  8,
    RIGHT_CANARY_IN_ARRAY_CHANGED   = 1 <<  9,
    STACK_HASH_CHANGED              = 1 << 10,
    DATA_HASH_CHANGED               = 1 << 11,
    POINTER_TO_STACK_INFO_IS_NULL   = 1 << 12,
    POINTER_RETURN_VALUE_POP_NULL   = 1 << 13
};

struct stack {
    TYPE_ELEMENT_STACK             *data;
    ssize_t                         size;
    ssize_t                         capacity;
    ssize_t                         error_code;
    struct debug_info              *info;
    FILE                           *logs_pointer;
    canary_t                        left_canary;
    canary_t                        right_canary;
    long long                       stack_hash;
    long long                       data_hash;
};

struct debug_info {
    ssize_t      line;
    const char  *name;
    const char  *file;
    const char  *func;
};
bool check_argc(int argc);

FILE *check_isopen (const char *file_name, const char *opening_mode);
bool check_isclose (FILE *file_pointer);

stack *get_pointer_stack(FILE *logs_pointer);

ssize_t stack_constructor(stack *stk);
ssize_t stack_destructor(stack *stk);

ssize_t push(stack *stk, TYPE_ELEMENT_STACK value);
ssize_t pop(stack *stk, TYPE_ELEMENT_STACK *return_value);

#endif  //STACK_H_INCLUDED
