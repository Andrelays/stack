#include "stack.h"
#include "myassert.h"

bool check_argc(int argc)
{
    if (argc != 2)
    {
        printf("ERROR! Incorrect numbers of coomand line arguments: %d.\n", argc);
        return false;
    }

    return true;
}

FILE *check_isopen (const char *file_name, const char *opening_mode)
{
    MYASSERT(file_name    != NULL,      NULL_POINTER_PASSED_TO_FUNC,    return NULL);
    MYASSERT(opening_mode != NULL,      NULL_POINTER_PASSED_TO_FUNC,    return NULL);
    MYASSERT(opening_mode != file_name, EQUAL_POINTERS_PASSED_TO_FUNC,  return NULL);

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
