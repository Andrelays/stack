#include "stack.h"
#include "myassert.h"

int main(int argc, const char *argv[])
{
    if(!check_argc(argc))
        return -1;

    const char *file_name_logs = argv[1];

    FILE *logs_pointer = check_isopen(file_name_logs, "w");
    MYASSERT(logs_pointer != NULL, COULD_NOT_OPEN_THE_FILE , return COULD_NOT_OPEN_THE_FILE);

    stack *stk = get_pointer_stack(logs_pointer);

    TYPE_ELEMENT_STACK return_value = 0;

    STACK_CONSTRUCTOR(stk);

    for (int i = 0; i < 6; i++)
        pop(stk, &return_value);

    for (int i = 0; i < 5; i++)
        push(stk, i + 1);

    for (int i = 0; i < 6; i++)
        pop(stk, &return_value);

    stack_destructor(stk);

    MYASSERT(check_isclose (logs_pointer), COULD_NOT_CLOSE_THE_FILE , return COULD_NOT_CLOSE_THE_FILE);

    return 0;
}
