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

#define STACK_DUMP(stk, logs_pointer)                                               \
do {                                                                                \
    stack_dump(stk, __LINE__, __FILE__, __PRETTY_FUNCTION__, logs_pointer);         \
} while(0)

#define CHECK_ERRORS(stk)                               \
do {                                                    \
    if ((stk->error_code = verify_stack(stk)))          \
        return stk->error_code;                         \
} while(0)

#define POISON                      192
#define INITIAL_CAPACITY_VALUE      1 //TODO use consts instead of defines
#define CAPACITY_MULTIPLIER         2
#define FORMAT_SPECIFIERS_STACK   "%d"

typedef int TYPE_ELEMENT_STACK;
typedef long long canary_t;

const canary_t value_left_canary_stack  = 100;
const canary_t value_right_canary_stack = 200;
const canary_t value_left_canary_array  = 700;
const canary_t value_right_canary_array = 900;


enum errors_code_stack {
    NO_ERROR                        =    0,
    POINTER_TO_STACK_IS_NULL        =    1,
    POINTER_TO_STACK_DATA_IS_NULL   =    2,
    SIZE_MORE_THAN_CAPACITY         =    4,
    CAPACITY_LESS_THAN_ZERO         =    8,
    SIZE_LESS_THAN_ZERO             =   16,
    SIZE_NULL_IN_POP                =   32,
    LEFT_CANARY_IN_STACK_CHANGED    =   64,
    RIGHT_CANARY_IN_STACK_CHANGED   =  128,
    LEFT_CANARY_IN_ARRAY_CHANGED    =  256,
    RIGHT_CANARY_IN_ARRAY_CHANGED   =  512, // TODO use bitwise shift
};

// TODO think about default values
struct stack {
    long long                       left_canary;
    TYPE_ELEMENT_STACK             *data;
    ssize_t                         size;
    ssize_t                         capacity;
    ssize_t                         line;
    ssize_t                         error_code;
    struct debug_info              *info;
    long long                       right_canary;
};

struct debug_info {
    ssize_t      line;
    const char  *name;
    const char  *file;
    const char  *func;
};

ssize_t stack_constructor(stack *stk);
ssize_t stack_destructor(stack *stk);

ssize_t push(stack *stk, TYPE_ELEMENT_STACK value);

ssize_t pop(stack *stk, TYPE_ELEMENT_STACK *return_value);

ssize_t verify_stack(stack *stk);

ssize_t check_capacity(stack *stk);

void stack_dump(stack *stk, ssize_t line, const char *file, const char *func, FILE *logs_pointer);
void print_debug_info(stack *stk, ssize_t line, const char *file, const char *func, FILE *logs_pointer);

void print_errors(stack *stk, FILE *logs_pointer);

FILE *check_isopen (const char *file_name, const char *opening_mode);
bool check_isclose (FILE *file_pointer);

ssize_t realloc_data(stack *stk); // TODO hide from publick interface: move to cpp + make static
canary_t *get_pointer_left_canary(TYPE_ELEMENT_STACK *data);
canary_t *get_pointer_right_canary(stack *stk);
size_t get_size_data (stack *stk);

stack *get_pointer_stack();

bool check_argc(int argc);

#endif  //STACK_H_INCLUDED
