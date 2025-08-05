#include "utils.h"
#include "error.h"

char *read_whole_file(char *path)
{
    uint64_t size = 0;
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        error_t err;
        sprintf(err.message, "Could not read source file `%s`", path);
        err.dont_show_line_col = 1;
        err.is_warning = 0;
        add_error(err);
        return (char *)0;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0)
    {
        error_t err;
        sprintf(err.message, "Could not read source file `%s`", path);
        err.dont_show_line_col = 1;
        err.is_warning = 0;
        add_error(err);
        return (char *)0;
    }
    char *buf = calloc(size + 1, 1);
    int _ = fread(buf, size, 1, f);
    fclose(f);
    return buf;
}

char *find_input_file(int argc, char **argv)
{
    char *res = "main.hyd";
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-o"))
        {
            ++i;
        }
        else if (!strcmp(argv[i], "-i"))
        {
            ++i;
            return argv[i];
        }
        else
        {
            res = argv[i];
        }
    }
    return res;
}