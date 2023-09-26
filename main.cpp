#include "stack.h"
#include "myassert.h"

int main(int argc, const char *argv[])
{
    if(!check_argc(argc))
        return -1;

    const char *file_name_logs = argv[1];

    stack *stk   = get_pointer_stack();
    stack *stk_1 = get_pointer_stack();

    FILE *logs_pointer = check_isopen(file_name_logs, "w");
    MYASSERT(logs_pointer != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);

    TYPE_ELEMENT_STACK return_value = 0;

    STACK_CONSTRUCTOR(stk);
    STACK_CONSTRUCTOR(stk_1);

    push(stk, 1);

    STACK_DUMP(stk, logs_pointer);

    push(stk, 1);

    STACK_DUMP(stk, logs_pointer);

    push(stk, 1);

    STACK_DUMP(stk, logs_pointer);
    
    push(stk, 1);

    STACK_DUMP(stk, logs_pointer);


    for (TYPE_ELEMENT_STACK index = 0; index < 20; index++)
    {
        if (pop(stk, &return_value) != 32 && pop(stk, &return_value) != 0)
            STACK_DUMP(stk, logs_pointer);

        stack_ok(stk, logs_pointer);
    }

    for (TYPE_ELEMENT_STACK index = 0; index < 10; index++)
    {
        if (push(stk_1, index))
            STACK_DUMP(stk_1, logs_pointer);

        stack_ok(stk_1, logs_pointer);
    }

    MYASSERT(check_isclose (logs_pointer), COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);

    stack_destructor(stk);
    stack_destructor(stk_1);

}
