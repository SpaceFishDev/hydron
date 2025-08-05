#include "error.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

error_t global_errors[1024];
uint64_t global_err_count = 0;

void add_error(error_t err)
{
    if (global_err_count >= 1024)
    {
        return;
    }
    global_errors[global_err_count] = err;
    ++global_err_count;
}

void put_error(error_t err)
{
    if (err.is_warning)
    {
        printf("WARNING: ");
        printf(COLOR_WARN "%s " COLOR_RESET, err.message);
        if (!err.dont_show_line_col)
        {
            printf("[ln %d : col %d]\n", err.line, err.column);
        }
        else
        {
            printf("\n");
        }
    }
    else
    {
        printf("ERROR: ");
        printf(COLOR_ERR "%s " COLOR_RESET, err.message);
        if (!err.dont_show_line_col)
        {
            printf("[ln %d : col %d]\n", err.line, err.column);
        }
        else
        {
            printf("\n");
        }
    }
}

void put_errors()
{
    bool found_fatal = false;
    for (uint64_t i = 0; i < global_err_count; ++i)
    {
        put_error(global_errors[i]);
        if (global_errors[i].is_warning == false)
        {
            found_fatal = true;
        }
    }
    if (found_fatal)
    {
        exit(-1);
    }
}

int error_count()
{
    return global_err_count;
}