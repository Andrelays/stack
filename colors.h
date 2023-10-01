/*!
\file
\brief Header файл с библиотекой цветов, использующихся для вывода
*/

#ifndef COLORS_H_INCLUDED
#define COLORS_H_INCLUDED

#define BLACK       "\033[1;30m"
#define RED         "\033[1;31m"
#define GREEN       "\033[1;32m"
#define YELLOW      "\033[1;33m"
#define BLUE        "\033[1;34m"
#define MAGENTA     "\033[1;35m"
#define CYAN        "\033[1;36m"
#define WHITE       "\033[1;37m"
#define RESET_COLOR "\033[1;0m"

#ifdef CONSOLE_OUTPUT

    #define LightGray   "\033[1;30m"
    #define Maroon      "\033[1;31m"
    #define Red         "\033[1;31m"
    #define Crimson     "\033[1;31m"
    #define Green       "\033[1;32m"
    #define MediumBlue  "\033[1;34m"
    #define BlueViolet  "\033[1;34m"
    #define DarkViolet  "\033[1;35m"
    #define Orange      "\033[1;33m"
    #define DarkMagenta "\033[1;35m"

    #define COLOR_PRINT(color, str, ...)                                        \
     do {                                                                       \
        if(Global_color_output)                                                 \
            fprintf(Global_logs_pointer, color str RESET_COLOR, __VA_ARGS__);   \
                                                                                \
        else                                                                    \
            fprintf(Global_logs_pointer, str, __VA_ARGS__);                     \
                                                                                \
    }  while(0)

#else

    #define COLOR_PRINT(color, str, ...)                                                    \
     do {                                                                                   \
        if(Global_color_output) fprintf(Global_logs_pointer, "<font color=" #color ">");    \
        fprintf(Global_logs_pointer, str, __VA_ARGS__);                                     \
        if(Global_color_output) fprintf(Global_logs_pointer, "</font>");                    \
    }  while(0)

#endif


#endif // COLORS_H_INCLUDED
