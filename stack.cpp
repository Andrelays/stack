#include "stack.h"
#include "myassert.h"
#include <stdlib.h>
#include <memory.h>

#ifdef DEBUG_OUTPUT_STACK_DUMP
#define OUTPUT_STACK_DUMP               \
do {                                    \
    if (error_code != NO_ERROR)         \
        STACK_DUMP(stk);                \
} while (0)

#define DECLARE_STACK_DUMP              \
do {                                    \

} while (0)

#else

#define OUTPUT_STACK_DUMP               \
do {                                    \
    ;                                   \
} while (0)
#endif

#ifdef DEBUG_OUTPUT_STACK_OK
#define OUTPUT_STACK_OK                 \
do {                                    \
    if (error_code == NO_ERROR)         \
        stack_ok(stk);                  \
} while (0)

#else

#define OUTPUT_STACK_OK                 \
do {                                    \
    ;                                   \
} while (0)
#endif

static ssize_t verify_stack(stack *stk);

static ssize_t check_capacity(stack *stk);
static ssize_t realloc_data(stack *stk);

static void stack_dump(stack *stk, ssize_t line, const char *file, const char *func);
static void print_errors(const stack *stk);
static void print_debug_info(const stack *stk, ssize_t line, const char *file, const char *func);

static void stack_ok(const stack *stk);

static canary_t *get_pointer_right_canary(const stack *stk);
static canary_t *get_pointer_left_canary(const stack *stk);
static size_t get_size_data (const stack *stk);

static ssize_t calculate_stack_hash(stack *stk);
static long long calculate_hash(void *array, ssize_t size);

static bool check_stack_hash(stack *stk);
static bool check_data_hash(stack *stk);

stack *get_pointer_stack(FILE *logs_pointer)
{
    struct stack *stk = (stack *) calloc(1, sizeof(stack));

    stk->data           = NULL;
    stk->size           = 0;
    stk->capacity       = 0;
    stk->error_code     = NO_ERROR;
    stk->info           = NULL;
    stk->logs_pointer   = logs_pointer;
    stk->left_canary    = VALUE_LEFT_CANARY_STACK;
    stk->right_canary   = VALUE_RIGHT_CANARY_STACK;
    stk->stack_hash     = 0;
    stk->data_hash      = 0;

    return stk;
}

ssize_t stack_constructor(stack *stk)
{
    stk->capacity = INITIAL_CAPACITY_VALUE;

    canary_t *array = (canary_t *) calloc(get_size_data(stk), 1);
    MYASSERT(array != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return 0);

    array[0] = VALUE_LEFT_CANARY_ARRAY;

    stk->data = (TYPE_ELEMENT_STACK *) (array + 1);

    *get_pointer_right_canary(stk) = VALUE_RIGHT_CANARY_ARRAY;

    stk->size = 0;

    calculate_stack_hash(stk);

    return (verify_stack(stk));
}

ssize_t stack_destructor(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    CHECK_ERRORS(stk);

    memset(stk->data, POISON, stk->capacity - 1);

    stk->size = -1;
    stk->capacity = -1;

    free(stk->info);
    free(get_pointer_left_canary(stk));
    free(stk);

    return NO_ERROR;
}

ssize_t push(stack *stk, TYPE_ELEMENT_STACK value)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    CHECK_ERRORS(stk);

    check_capacity(stk);

    (stk->data)[stk->size++] = value;

    calculate_stack_hash(stk);

    return (verify_stack(stk));
}

ssize_t pop(stack *stk, TYPE_ELEMENT_STACK *return_value)
{
    MYASSERT(return_value != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_RETURN_VALUE_POP_NULL);
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    CHECK_ERRORS(stk);

    if (stk->size == 0)
        return (SIZE_NULL_IN_POP);

    --stk->size;

    *return_value = (stk->data)[stk->size];
    (stk->data)[stk->size] = POISON;

    calculate_stack_hash(stk);

    check_capacity(stk);

    return (verify_stack(stk));
}

ssize_t check_capacity(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    CHECK_ERRORS(stk);

    if (stk->size >= stk->capacity)
    {
        stk->capacity *= CAPACITY_MULTIPLIER;

        realloc_data(stk);
    }

    else if ((stk->size + 1) * CAPACITY_MULTIPLIER * CAPACITY_MULTIPLIER <= stk->capacity)
    {
        stk->capacity /= CAPACITY_MULTIPLIER;

        realloc_data(stk);
    }

    return (verify_stack(stk));
}

ssize_t realloc_data(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);

    canary_t *array = (canary_t *) realloc(get_pointer_left_canary(stk), get_size_data (stk));
    MYASSERT(array != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return 0);

    stk->data = (TYPE_ELEMENT_STACK *) (array + 1);

    *get_pointer_right_canary(stk) = VALUE_RIGHT_CANARY_ARRAY;

    for (ssize_t index = stk->size; index < stk->capacity; index++)
    (stk->data)[index] = POISON;

    calculate_stack_hash(stk);

    return (verify_stack(stk));
}

ssize_t verify_stack(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    ssize_t error_code = NO_ERROR;

    #define SUMMARIZE_ERRORS_(condition, added_error)   \
    do {                                                \
        if((condition))                                 \
            error_code += added_error;                  \
    } while(0)

    SUMMARIZE_ERRORS_(!stk,                                                             POINTER_TO_STACK_IS_NULL);
    SUMMARIZE_ERRORS_(!stk->data,                                                       POINTER_TO_STACK_DATA_IS_NULL);
    SUMMARIZE_ERRORS_(!stk->info,                                                       POINTER_TO_STACK_INFO_IS_NULL);
    SUMMARIZE_ERRORS_(stk->size > stk->capacity,                                        SIZE_MORE_THAN_CAPACITY);
    SUMMARIZE_ERRORS_(stk->capacity < 0,                                                CAPACITY_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(stk->size     < 0,                                                SIZE_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(stk->left_canary  != VALUE_LEFT_CANARY_STACK,                     LEFT_CANARY_IN_STACK_CHANGED);
    SUMMARIZE_ERRORS_(stk->right_canary != VALUE_RIGHT_CANARY_STACK,                    RIGHT_CANARY_IN_STACK_CHANGED);
    SUMMARIZE_ERRORS_(*get_pointer_left_canary(stk)  != VALUE_LEFT_CANARY_ARRAY,        LEFT_CANARY_IN_ARRAY_CHANGED);
    SUMMARIZE_ERRORS_(*get_pointer_right_canary(stk) != VALUE_RIGHT_CANARY_ARRAY,       RIGHT_CANARY_IN_ARRAY_CHANGED);
    SUMMARIZE_ERRORS_(!check_stack_hash(stk),                                           STACK_HASH_CHANGED);
    SUMMARIZE_ERRORS_(!check_data_hash(stk),                                            DATA_HASH_CHANGED);

    #undef SUMMARIZE_ERRORS_

    OUTPUT_STACK_DUMP;
    OUTPUT_STACK_OK;

    stk->error_code = error_code;

    return stk->error_code;
}

void stack_dump(stack *stk, ssize_t line, const char *file, const char *func)
{
    MYASSERT(stk                != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->data          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->info          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->logs_pointer  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(file               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    print_errors(stk);

    print_debug_info(stk, line, file, func);

    for (ssize_t index = 0; index < stk->capacity; index++)
    {
        if ((stk->data)[index] == POISON)
            fprintf(stk->logs_pointer, "\t\t [%ld] = " FORMAT_SPECIFIERS_STACK " (POISON) [%p]\n", index, POISON, stk->data + index);

        else
            fprintf(stk->logs_pointer, "\t\t*[%ld] = " FORMAT_SPECIFIERS_STACK " [%p]\n", index, (stk->data)[index], stk->data + index);
    }

    fprintf(stk->logs_pointer,   "\t\t [right_canary] = %lld (reference_value = %lld) [%p]\n"
                                 "\t}\n"
                                 "}\n\n",
                *(get_pointer_right_canary(stk)), VALUE_RIGHT_CANARY_ARRAY, get_pointer_right_canary(stk));

}

void print_debug_info(const stack *stk, ssize_t line, const char *file, const char *func)
{
    MYASSERT(stk                != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->data          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->info          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->logs_pointer  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(file               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    fprintf(stk->logs_pointer,   "stack[%p]\n"
                                 "\"%s\" from %s(%ld) %s\n"
                                 "called from %s(%ld) %s\n"
                                 "{\n"
                                 "\tleft_canary = %lld (reference_value = %lld)\n"
                                 "\tsize = %ld\n"
                                 "\tcapacity = %ld\n"
                                 "\tdata[%p]\n"
                                 "\tright_canary = %lld (reference_value = %lld)\n"
                                 "\t{\n"
                                 "\t\t [left_canary] = %lld (reference_value = %lld) [%p]\n",
            stk, stk->info->name, stk->info->file, stk->info->line, stk->info->func,
            file, line, func, stk->left_canary, VALUE_LEFT_CANARY_STACK, stk->size, stk->capacity, stk->data, stk->right_canary,
            VALUE_RIGHT_CANARY_STACK, *(get_pointer_left_canary(stk)), VALUE_LEFT_CANARY_ARRAY, get_pointer_left_canary(stk));
}

void print_errors(const stack *stk)
{
    MYASSERT(stk               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->data         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    #define GET_ERRORS_(error)                                  \
    do {                                                        \
        if(stk->error_code & error)                             \
            fprintf(stk->logs_pointer, "Errors: %s\n",#error);  \
    } while(0)

   GET_ERRORS_(POINTER_TO_STACK_IS_NULL);
   GET_ERRORS_(POINTER_TO_STACK_DATA_IS_NULL);
   GET_ERRORS_(SIZE_MORE_THAN_CAPACITY);
   GET_ERRORS_(CAPACITY_LESS_THAN_ZERO);
   GET_ERRORS_(SIZE_LESS_THAN_ZERO);
   GET_ERRORS_(SIZE_NULL_IN_POP);
   GET_ERRORS_(LEFT_CANARY_IN_STACK_CHANGED);
   GET_ERRORS_(RIGHT_CANARY_IN_STACK_CHANGED);
   GET_ERRORS_(LEFT_CANARY_IN_ARRAY_CHANGED);
   GET_ERRORS_(RIGHT_CANARY_IN_ARRAY_CHANGED);
   GET_ERRORS_(STACK_HASH_CHANGED);
   GET_ERRORS_(DATA_HASH_CHANGED);

   #undef GET_ERRORS_
}

void stack_ok(const stack *stk)
{
    MYASSERT(stk               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->data         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    fprintf(stk->logs_pointer,   "%s\n"
                                 "{\n",
                        stk->info->name);

    for (ssize_t index = 0; index < stk->capacity; index++)
        if ((stk->data)[index] != POISON)
            fprintf(stk->logs_pointer, "\t[%ld] = " FORMAT_SPECIFIERS_STACK "\n", index, (stk->data)[index]);

    fprintf(stk->logs_pointer, "}\n\n");
}

canary_t *get_pointer_left_canary(const stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    return (((canary_t *) stk->data) - 1);
}

canary_t *get_pointer_right_canary(const stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    return ((canary_t *) (((char *) get_pointer_left_canary(stk)) + get_size_data(stk) - sizeof(canary_t)));
}

size_t get_size_data(const stack *stk)
{
    MYASSERT(stk       != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(stk->info != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    return (sizeof(TYPE_ELEMENT_STACK) * stk->capacity + 3 * sizeof(canary_t) -
           ((stk->capacity * sizeof(TYPE_ELEMENT_STACK)) % sizeof(canary_t)));
}

ssize_t calculate_stack_hash(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    stk->stack_hash = 0;
    stk->data_hash  = 0;
    stk->stack_hash = calculate_hash(stk, sizeof(*stk));
    stk->data_hash  = calculate_hash(get_pointer_left_canary(stk), get_size_data(stk));

    return NO_ERROR;
}

long long calculate_hash(void *array, ssize_t size)
{
    MYASSERT(array != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
    MYASSERT(size > 0,      NEGATIVE_VALUE_SIZE_T,       return 0);

    long long hash = (long long) array;

    for(ssize_t counter = 0; counter < size; counter++)
    {
        hash += 3 * *((char *) array + counter);
    }

    return hash;
}

bool check_stack_hash(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    long long hash       = stk->stack_hash;
    long long hash_data  = stk->data_hash;

    stk->data_hash  = 0;
    stk->stack_hash = 0;

    if (hash != calculate_hash(stk, sizeof(*stk)))
    {
        stk->stack_hash = hash;
        stk->data_hash = hash_data;

        return false;
    }

    stk->stack_hash = hash;
    stk->data_hash = hash_data;

    return true;
}

bool check_data_hash(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    if (stk->data_hash != calculate_hash(get_pointer_left_canary(stk), get_size_data(stk)))
        return false;

    return true;
}
