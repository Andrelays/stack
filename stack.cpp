#include "stack.h"
#include "myassert.h"
#include <stdlib.h>
#include <memory.h>

ssize_t stack_constructor(stack *stk)
{
    stk->capacity = INITIAL_CAPACITY_VALUE;

    canary_t *array = (canary_t *) calloc(get_size_data(stk), 1);
    MYASSERT(array != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return 0);

    array[0] = value_left_canary_array;

    stk->data = (TYPE_ELEMENT_STACK *) (array + 1);

    *get_pointer_right_canary(stk) = value_right_canary_array;

    stk->size = 0;

    return (verify_stack(stk));
}

ssize_t stack_destructor(stack *stk)
{
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
    CHECK_ERRORS(stk);

    check_capacity(stk);

    (stk->data)[stk->size++] = value;

    return (verify_stack(stk));
}

ssize_t pop(stack *stk, TYPE_ELEMENT_STACK *return_value)
{
    MYASSERT(return_value != NULL, NULL_POINTER_PASSED_TO_FUNC, return 0);

    CHECK_ERRORS(stk);

    if (stk->size == 0)
        return (SIZE_NULL_IN_POP);

    --stk->size;

    *return_value = (stk->data)[stk->size];
    (stk->data)[stk->size] = POISON;

    check_capacity(stk);

    return (verify_stack(stk));
}

ssize_t verify_stack(stack *stk)
{
    #define SUMMARIZE_ERRORS_(condition, added_error)   \
    do {                                                \
        if((condition))                                 \
            stk->error_code += added_error;             \
    } while(0)

    SUMMARIZE_ERRORS_(!stk,                                                             POINTER_TO_STACK_IS_NULL);
    SUMMARIZE_ERRORS_(!stk->data,                                                       POINTER_TO_STACK_DATA_IS_NULL);
    SUMMARIZE_ERRORS_(stk->size > stk->capacity,                                        SIZE_MORE_THAN_CAPACITY);
    SUMMARIZE_ERRORS_(stk->capacity < 0,                                                CAPACITY_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(stk->size     < 0,                                                SIZE_LESS_THAN_ZERO);
    SUMMARIZE_ERRORS_(stk->left_canary  != value_left_canary_stack,                     LEFT_CANARY_IN_STACK_CHANGED);
    SUMMARIZE_ERRORS_(stk->right_canary != value_right_canary_stack,                    RIGHT_CANARY_IN_STACK_CHANGED);
    SUMMARIZE_ERRORS_(*get_pointer_left_canary(stk)  != value_left_canary_array,        LEFT_CANARY_IN_ARRAY_CHANGED);
    SUMMARIZE_ERRORS_(*get_pointer_right_canary(stk) != value_right_canary_array,       RIGHT_CANARY_IN_ARRAY_CHANGED);

    #undef SUMMARIZE_ERRORS_

    return stk->error_code;
}

ssize_t check_capacity(stack *stk)
{
    CHECK_ERRORS(stk);

    if (stk->size >= stk->capacity)
    {
        stk->capacity *= CAPACITY_MULTIPLIER;//TODO ERROR CAPACITY

        realloc_data(stk);

        for (ssize_t index = stk->size; index < stk->capacity; index++)
            (stk->data)[index] = POISON;
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

    canary_t *array = (canary_t *) realloc(get_pointer_left_canary(stk), get_size_data (stk));
    MYASSERT(array != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return 0);

    stk->data = (TYPE_ELEMENT_STACK *) (array + 1);

    *get_pointer_right_canary(stk) = value_right_canary_array;

    return (verify_stack(stk));
}

void stack_dump(stack *stk, ssize_t line, const char *file, const char *func, FILE *logs_pointer)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(file         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    print_errors(stk, logs_pointer);

    print_debug_info(stk, line, file, func, logs_pointer);

    for (ssize_t index = 0; index < stk->capacity; index++)
    {
        if ((stk->data)[index] == POISON)
            fprintf(logs_pointer, "\t\t [%ld] =" FORMAT_SPECIFIERS_STACK " (POISON) [%p]\n", index, POISON, stk->data + index);

        else
            fprintf(logs_pointer, "\t\t*[%ld] =" FORMAT_SPECIFIERS_STACK " [%p]\n", index, (stk->data)[index], stk->data + index);
    }

    fprintf(logs_pointer,   "\t\t [right_canary] = %lld (reference_value = %lld) [%p]\n"
                            "\t}\n"
                            "}\n\n",
                *(get_pointer_right_canary(stk)), value_right_canary_array, get_pointer_right_canary(stk));

}

void stack_ok(stack *stk, FILE *logs_pointer)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    fprintf(logs_pointer,   "%s\n"
                            "{\n",
                        stk->info->name);

    for (ssize_t index = 0; index < stk->capacity; index++)
    {
        if ((stk->data)[index] == POISON)
            fprintf(logs_pointer, "\t[%ld] =" FORMAT_SPECIFIERS_STACK " (POISON)\n", index, POISON);

        else
            fprintf(logs_pointer, "\t[%ld] =" FORMAT_SPECIFIERS_STACK "\n", index, (stk->data)[index]);
    }

    fprintf(logs_pointer, "}\n\n");
}

void print_debug_info(stack *stk, ssize_t line, const char *file, const char *func, FILE *logs_pointer)
{
    MYASSERT(stk          != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(stk->data    != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(file         != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    fprintf(logs_pointer,   "stack[%p]\n"
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
            file, line, func, stk->left_canary, value_left_canary_stack, stk->size, stk->capacity, stk->data, stk->right_canary,
            value_right_canary_stack, *(get_pointer_left_canary(stk)), value_left_canary_array, get_pointer_left_canary(stk));
}

FILE *check_isopen (const char *file_name, const char *opening_mode)
{
    MYASSERT(file_name != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    FILE *file_pointer = NULL;

    if ((file_pointer = fopen (file_name, opening_mode)) == NULL || ferror (file_pointer))
        printf("ERROR! Could not open the file \"%s\"!\n", file_name);

    return file_pointer;
}

bool check_isclose (FILE *file_pointer)
{
    MYASSERT(file_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    if (fclose(file_pointer))
    {
        printf("ERROR! Could not close the file\n");
        return false;
    }

    return true;
}

void print_errors(stack *stk, FILE *logs_pointer)
{
    MYASSERT(logs_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    #define GET_ERRORS_(error)                              \
    do {                                                    \
        if(stk->error_code & error)                         \
            fprintf(logs_pointer, "Errors: %s\n",#error);   \
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


   #undef GET_ERRORS_
}


canary_t *get_pointer_left_canary(stack *stk)
{
    return (((canary_t *) stk->data) - 1);
}

canary_t *get_pointer_right_canary(stack *stk)
{
    return ((canary_t *) (((char *) get_pointer_left_canary(stk)) + get_size_data(stk) - sizeof(canary_t)));
}

size_t get_size_data(stack *stk)
{
    return (sizeof(TYPE_ELEMENT_STACK) * stk->capacity + 3 * sizeof(canary_t) -
           ((stk->capacity * sizeof(TYPE_ELEMENT_STACK)) % sizeof(canary_t)));
}

stack *get_pointer_stack()
{
    struct stack *stk = (stack *) calloc(1, sizeof(stack));

    stk->left_canary    = value_left_canary_stack;
    stk->data           = NULL;
    stk->size           = 0;
    stk->capacity       = 0;
    stk->line           = 0;
    stk->info           = NULL;
    stk->right_canary   = value_right_canary_stack;

    return stk;
}

bool check_argc(int argc)
{
    if (argc != 2)
    {
        printf("ERROR! Incorrect numbers of coomand line arguments: %d.\n", argc);
        return false;
    }

    return true;
}
