/*!
\file
\brief Header файл с макросом MYASSERT и списком возможных ошибок
*/

#ifndef MYASSERT_H_INCLUDED
#define MYASSERT_H_INCLUDED

#include"colors.h"

#ifdef DEBUG
/// Макрос, используемый для нахождения логических ошибкок, похож на assert, но в случае ошибки завершает функцию, а не программу, печатает файл, строку, функцию, в которой произошла ошибка и номер ошибки
#define MYASSERT(condition, error_code, finish)                                                                     \
 do {                                                                                                               \
    if (!(condition))                                                                                               \
    {                                                                                                               \
        printf(MAGENTA "suddenly: Logical error: Error code: %s\n", #error_code);                                   \
        printf("File: %s\n" "line: %d\n" "Function: %s\n" RESET_COLOR, __FILE__, __LINE__, __PRETTY_FUNCTION__);    \
                                                                                                                    \
        finish;                                                                                                     \
    }                                                                                                               \
}  while(0)

#else
#define MYASSERT(condition, error_code, finish)

#endif

/// Список возможных ошибок, которые печатает MYASSERT
enum errors_code {
    ASSERT_NO_ERROR                 = 0,
    NON_FINITE_NUM_PASSED_TO_FUNC   = 1,///< Указывает, что в функцию было передано не конечное число
    NULL_POINTER_PASSED_TO_FUNC     = 2,///< Указывает, что в функцию был передан 0 указатель
    EQUAL_POINTERS_PASSED_TO_FUNC   = 3,///< Указывает, что в функцию были переданы равные указатели
    GOING_BEYOUND_BOUNDARY_ARRAY    = 4,///< Указывает, что индекс находится за границей массива
    NEGATIVE_VALUE_SIZE_T           = 5,///< Указывает, что указан отрицательный размер size_t
    FAILED_TO_ALLOCATE_DYNAM_MEMOR  = 6,
    COULD_NOT_OPEN_THE_FILE         = 7,
    COULD_NOT_CLOSE_THE_FILE        = 8,
    HASH_HAS_BEEN_CHANGED           = 9,
    INCORRECT_NUMBER_OF_ARGC        = 10
};


#endif // MYASSERT_H_INCLUDED
