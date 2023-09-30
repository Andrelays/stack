#include "stack.h"
#include "myassert.h"
#include <stdlib.h>
#include <memory.h>

#ifdef INCREASED_LEVEL_OF_PROTECTION

    #define ON_INCREASED_LEVEL_OF_PROTECTION(...)   __VA_ARGS__

#else

    #define ON_INCREASED_LEVEL_OF_PROTECTION(...)

#endif

#ifdef DEBUG_OUTPUT_STACK_DUMP

    #define IF_ON_STACK_DUMP(...)   __VA_ARGS__

    #define GET_ERRORS_(error)                                  \
    do {                                                        \
        if(stk->error_code & error)                             \
            fprintf(stk->logs_pointer, "Errors: %s\n",#error);  \
    } while(0)

    #define STACK_DUMP(stk)                                                 \
    do {                                                                    \
        stack_dump(stk, __LINE__, __FILE__, __PRETTY_FUNCTION__);           \
    } while(0)

#else

    #define IF_ON_STACK_DUMP(...)

#endif

#ifdef DEBUG_OUTPUT_STACK_OK

    #define IF_ON_STACK_OK(...)     __VA_ARGS__

#else

    #define IF_ON_STACK_OK(...)

#endif

#define CHECK_ERRORS(stk)                                       \
do {                                                            \
    if (((stk)->error_code = verify_stack(stk)) != NO_ERROR)    \
        return (stk)->error_code;                               \
} while(0)

IF_ON_CANARY_PROTECT (const canary_t VALUE_LEFT_CANARY_STACK  = 0xDEDDAD);
IF_ON_CANARY_PROTECT (const canary_t VALUE_RIGHT_CANARY_STACK = 0xDEDBED);
IF_ON_CANARY_PROTECT (const canary_t VALUE_LEFT_CANARY_ARRAY  = 0xDEDDED);
IF_ON_CANARY_PROTECT (const canary_t VALUE_RIGHT_CANARY_ARRAY = 0xDEDBAD);

static ssize_t verify_stack(stack *stk);

static ssize_t check_capacity(stack *stk);
static ssize_t realloc_data(stack *stk);

IF_ON_STACK_DUMP(static void stack_dump(stack *stk, ssize_t line, const char *file, const char *func));
IF_ON_STACK_DUMP(static void print_debug_info(const stack *stk, ssize_t line, const char *file, const char *func));
IF_ON_STACK_DUMP(static void print_errors(const stack *stk));

IF_ON_STACK_OK(static void stack_ok(const stack *stk));

IF_ON_CANARY_PROTECT(static canary_t *get_pointer_right_canary(const stack *stk));
IF_ON_CANARY_PROTECT(static canary_t *get_pointer_left_canary(const stack *stk));
IF_ON_CANARY_PROTECT(static size_t get_size_data (const stack *stk));

IF_ON_HASH_PROTECT(static ssize_t calculate_stack_hash(stack *stk));
IF_ON_HASH_PROTECT(static uint32_t calculate_hash(void *array, ssize_t size));

IF_ON_HASH_PROTECT(static bool check_stack_hash(stack *stk));
IF_ON_HASH_PROTECT(static bool check_data_hash(stack *stk));

stack *get_pointer_stack(FILE *logs_pointer)
{
    MYASSERT(logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    struct stack *stk = (stack *) calloc(1, sizeof(stack));

    stk->data           = NULL;
    stk->size           = 0;
    stk->capacity       = 0;
    stk->error_code     = NO_ERROR;
    stk->info           = NULL;
    stk->logs_pointer   = logs_pointer;

    IF_ON_CANARY_PROTECT(stk->left_canary  = VALUE_LEFT_CANARY_STACK);
    IF_ON_CANARY_PROTECT(stk->right_canary = VALUE_RIGHT_CANARY_STACK);

    IF_ON_HASH_PROTECT(stk->stack_hash = 0);
    IF_ON_HASH_PROTECT(stk->data_hash = 0);

    return stk;
}

ssize_t stack_constructor(stack *stk)
{
    MYASSERT(stk != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    stk->capacity = INITIAL_CAPACITY_VALUE;

    IF_ON_CANARY_PROTECT
    (
        canary_t *array = (canary_t *) calloc(get_size_data(stk), 1);
        MYASSERT(array != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_STACK_DATA_IS_NULL);
    )

    ELSE_IF_OFF_CANARY_PROTECT
    (
        stk->data = (TYPE_ELEMENT_STACK *) calloc(stk->capacity, sizeof(TYPE_ELEMENT_STACK));
        MYASSERT(stk->data != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return POINTER_TO_STACK_DATA_IS_NULL);
    )

    IF_ON_CANARY_PROTECT(array[0] = VALUE_LEFT_CANARY_ARRAY);

    IF_ON_CANARY_PROTECT(stk->data = (TYPE_ELEMENT_STACK *) (array + 1));

    IF_ON_CANARY_PROTECT(*get_pointer_right_canary(stk) = VALUE_RIGHT_CANARY_ARRAY);

    stk->size = 0;

    IF_ON_HASH_PROTECT(calculate_stack_hash(stk));

    return (verify_stack(stk));
}

ssize_t stack_destructor(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    CHECK_ERRORS(stk);

    stk->size = -1;
    stk->capacity = -1;

    IF_ON_CANARY_PROTECT
    (
        memset(get_pointer_left_canary(stk), POISON, get_size_data(stk));
        free(get_pointer_left_canary(stk));
    )

    ELSE_IF_OFF_CANARY_PROTECT
    (
        memset(stk->data, POISON, stk->capacity);
        free(stk->data);
    )

    stk->data = NULL;

    free(stk->info);
    stk->info = NULL;

    free(stk);
    stk = NULL;

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

    IF_ON_HASH_PROTECT(calculate_stack_hash(stk));

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

    IF_ON_HASH_PROTECT(calculate_stack_hash(stk));

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

    IF_ON_CANARY_PROTECT
    (
        canary_t *array = (canary_t *) realloc(get_pointer_left_canary(stk), get_size_data (stk));
        MYASSERT(array != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return 0);
    )

    ELSE_IF_OFF_CANARY_PROTECT
    (
        stk->data = (TYPE_ELEMENT_STACK *) realloc(stk->data, stk->capacity * sizeof(TYPE_ELEMENT_STACK));
        MYASSERT(stk->data != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return 0);
    )

    IF_ON_CANARY_PROTECT(stk->data = (TYPE_ELEMENT_STACK *) (array + 1));

    IF_ON_CANARY_PROTECT(*get_pointer_right_canary(stk) = VALUE_RIGHT_CANARY_ARRAY);

    for (ssize_t index = stk->size; index < stk->capacity; index++)
        (stk->data)[index] = POISON;

    IF_ON_HASH_PROTECT(calculate_stack_hash(stk));

    return (verify_stack(stk));
}

ssize_t verify_stack(stack *stk)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
    MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

    ON_INCREASED_LEVEL_OF_PROTECTION(MYASSERT(check_stack_hash(stk), HASH_HAS_BEEN_CHANGED, return STACK_HASH_CHANGED));

    ssize_t error_code = NO_ERROR;

    #define SUMMARIZE_ERRORS_(condition, added_error)   \
    do {                                                \
        if((condition))                                 \
            error_code += added_error;                  \
    } while(0)

    SUMMARIZE_ERRORS_(!stk,                      POINTER_TO_STACK_IS_NULL);
    SUMMARIZE_ERRORS_(!stk->data,                POINTER_TO_STACK_DATA_IS_NULL);
    SUMMARIZE_ERRORS_(!stk->info,                POINTER_TO_STACK_INFO_IS_NULL);
    SUMMARIZE_ERRORS_(stk->size > stk->capacity, SIZE_MORE_THAN_CAPACITY);
    SUMMARIZE_ERRORS_(stk->capacity < 0,         CAPACITY_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(stk->size     < 0,         SIZE_LESS_THAN_ZERO);

    IF_ON_CANARY_PROTECT(SUMMARIZE_ERRORS_(stk->left_canary               != VALUE_LEFT_CANARY_STACK,  LEFT_CANARY_IN_STACK_CHANGED));
    IF_ON_CANARY_PROTECT(SUMMARIZE_ERRORS_(stk->right_canary              != VALUE_RIGHT_CANARY_STACK, RIGHT_CANARY_IN_STACK_CHANGED));
    IF_ON_CANARY_PROTECT(SUMMARIZE_ERRORS_(*get_pointer_left_canary(stk)  != VALUE_LEFT_CANARY_ARRAY,  LEFT_CANARY_IN_ARRAY_CHANGED));
    IF_ON_CANARY_PROTECT(SUMMARIZE_ERRORS_(*get_pointer_right_canary(stk) != VALUE_RIGHT_CANARY_ARRAY, RIGHT_CANARY_IN_ARRAY_CHANGED));

    IF_ON_HASH_PROTECT(SUMMARIZE_ERRORS_(!check_stack_hash(stk), STACK_HASH_CHANGED));
    IF_ON_HASH_PROTECT(SUMMARIZE_ERRORS_(!check_data_hash(stk),  DATA_HASH_CHANGED));

    #undef SUMMARIZE_ERRORS_

    IF_ON_STACK_DUMP
    (
        if (error_code != NO_ERROR)
            STACK_DUMP(stk);
    )

    IF_ON_STACK_OK
    (
        if (error_code == NO_ERROR)
            stack_ok(stk);
    )

    stk->error_code = error_code;

    return stk->error_code;
}

IF_ON_STACK_DUMP
(
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

        IF_ON_CANARY_PROTECT
        (
            fprintf(stk->logs_pointer,  "\t\t [right_canary] = %lld (reference_value = %lld) [%p]\n"
                                        "\t}\n"
                                        "}\n\n",
                                *(get_pointer_right_canary(stk)), VALUE_RIGHT_CANARY_ARRAY, get_pointer_right_canary(stk));
        )

        ELSE_IF_OFF_CANARY_PROTECT
        (
            fprintf(stk->logs_pointer,  "\t}\n"
                                        "}\n\n");
        )
    }
)

IF_ON_STACK_DUMP
(
    void print_debug_info(const stack *stk, ssize_t line, const char *file, const char *func)
    {
        MYASSERT(stk                != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->data          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->info          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->logs_pointer  != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(file               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        IF_ON_CANARY_PROTECT
        (
            fprintf(stk->logs_pointer,  "stack[%p]\n"
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
        )

        ELSE_IF_OFF_CANARY_PROTECT
        (
            fprintf(stk->logs_pointer,  "stack[%p]\n"
                                        "\"%s\" from %s(%ld) %s\n"
                                        "called from %s(%ld) %s\n"
                                        "{\n"
                                        "\tsize = %ld\n"
                                        "\tcapacity = %ld\n"
                                        "\tdata[%p]\n"
                                        "\t{\n",
                stk, stk->info->name, stk->info->file, stk->info->line, stk->info->func,
                file, line, func, stk->size, stk->capacity, stk->data);
        )
    }
)

IF_ON_STACK_DUMP
(
    void print_errors(const stack *stk)
    {
        MYASSERT(stk               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->data         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        IF_ON_CANARY_PROTECT (GET_ERRORS_(LEFT_CANARY_IN_STACK_CHANGED));
        IF_ON_CANARY_PROTECT (GET_ERRORS_(RIGHT_CANARY_IN_STACK_CHANGED));
        IF_ON_CANARY_PROTECT (GET_ERRORS_(LEFT_CANARY_IN_ARRAY_CHANGED));
        IF_ON_CANARY_PROTECT (GET_ERRORS_(RIGHT_CANARY_IN_ARRAY_CHANGED));

        GET_ERRORS_(POINTER_TO_STACK_IS_NULL);
        GET_ERRORS_(POINTER_TO_STACK_DATA_IS_NULL);
        GET_ERRORS_(SIZE_MORE_THAN_CAPACITY);
        GET_ERRORS_(CAPACITY_LESS_THAN_ZERO);
        GET_ERRORS_(SIZE_LESS_THAN_ZERO);
        GET_ERRORS_(SIZE_NULL_IN_POP);

        IF_ON_HASH_PROTECT (GET_ERRORS_(STACK_HASH_CHANGED));
        IF_ON_HASH_PROTECT (GET_ERRORS_(DATA_HASH_CHANGED));
    }
)

IF_ON_STACK_OK
(
    void stack_ok(const stack *stk)
    {
        MYASSERT(stk               != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->data         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->info         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
        MYASSERT(stk->logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

        fprintf(stk->logs_pointer,   "%s\n"
                                     "{\n",
                            stk->info->name);

        for (ssize_t index = 0; index < stk->size; index++)
                fprintf(stk->logs_pointer, "\t[%ld] = " FORMAT_SPECIFIERS_STACK "\n", index, (stk->data)[index]);

        fprintf(stk->logs_pointer, "}\n\n");
    }
)

IF_ON_CANARY_PROTECT
(
    canary_t *get_pointer_left_canary(const stack *stk)
    {
        MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
        MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
        MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

        return (((canary_t *) stk->data) - 1);
    }
)

IF_ON_CANARY_PROTECT
(
    canary_t *get_pointer_right_canary(const stack *stk)
    {
        MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
        MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
        MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

        return ((canary_t *) (((char *) get_pointer_left_canary(stk)) + get_size_data(stk) - sizeof(canary_t)));
    }
)

IF_ON_CANARY_PROTECT
(
    size_t get_size_data(const stack *stk)
    {
        MYASSERT(stk       != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
        MYASSERT(stk->info != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

        return (sizeof(TYPE_ELEMENT_STACK) * stk->capacity + 3 * sizeof(canary_t) -
                ((stk->capacity * sizeof(TYPE_ELEMENT_STACK)) % sizeof(canary_t)));
    }
)

IF_ON_HASH_PROTECT
(
    ssize_t calculate_stack_hash(stack *stk)
    {
        MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_IS_NULL);
        MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_DATA_IS_NULL);
        MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_STACK_INFO_IS_NULL);

        stk->stack_hash = 0;
        stk->data_hash  = 0;

        IF_ON_CANARY_PROTECT
        (
            stk->stack_hash = calculate_hash(stk, sizeof(*stk));
            stk->data_hash  = calculate_hash(get_pointer_left_canary(stk), get_size_data(stk));
        )

        ELSE_IF_OFF_CANARY_PROTECT
        (
            stk->stack_hash = calculate_hash(stk, sizeof(*stk));
            stk->data_hash  = calculate_hash(stk->data, stk->capacity);
        )

        return NO_ERROR;
    }
)

IF_ON_HASH_PROTECT
(
    uint32_t calculate_hash(void *array, ssize_t size)
    {
        MYASSERT(array != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);
        MYASSERT(size > 0,      NEGATIVE_VALUE_SIZE_T,       return 0);

        uint32_t hash = (uint32_t)((size_t) array);

        for(ssize_t counter = 0; counter < size; counter++)
        {
            hash += *((char *) array + counter);
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }

        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);

        return hash;
    }
)

IF_ON_HASH_PROTECT
(
    bool check_stack_hash(stack *stk)
    {
        MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
        MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
        MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

        uint32_t hash       = stk->stack_hash;
        uint32_t hash_data  = stk->data_hash;

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
)

IF_ON_HASH_PROTECT
(
    bool check_data_hash(stack *stk)
    {
        MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
        MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
        MYASSERT(stk->info    != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

        IF_ON_CANARY_PROTECT
        (
            if (stk->data_hash != calculate_hash(get_pointer_left_canary(stk), get_size_data(stk)))
                return false;
        )

        ELSE_IF_OFF_CANARY_PROTECT
        (
            if (stk->data_hash != calculate_hash(stk->data, stk->capacity))
                return false;
        )

        return true;
    }
)
