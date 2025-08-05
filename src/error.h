#ifndef ERROR_H
#define ERROR_H

typedef struct
{
    int is_warning;
    char message[512];
    int line, column;
    int dont_show_line_col;
} error_t;

void add_error(error_t err);

void put_error(error_t err);

void put_errors();

int error_count();

#define COLOR_RESET "\033[0m"
#define COLOR_NODE "\033[1;34m"
#define COLOR_TYPE "\033[1;32m"
#define COLOR_TOKEN "\033[1;36m"
#define COLOR_ERR "\033[1;31m"
#define COLOR_WARN "\033[1;33m"

#endif