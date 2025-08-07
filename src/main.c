#include "utils.h"
#include "error.h"
#include "lexer.h"
#include "bytecode.h"
#include "compiler.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

const int logging = 1;

token_t *lex_all(char *src, int *_num)
{
    lexer_t lexer;
    lexer.src = src;
    lexer.line = 1;
    lexer.column = 0;
    lexer.pos = 0;
    token_t *tokens = malloc(sizeof(token_t));
    uint64_t num = 0;
    while (1)
    {
        token_t t = lex(&lexer);
        if (t.type == END_OF_FILE)
        {
            break;
        }
        ++num;
        tokens = realloc(tokens, sizeof(token_t) * num);
        tokens[num - 1] = t;
    }
    if (error_count())
    {
        put_errors();
    }
    *_num = num;
    return tokens;
}

char *get_output(int argc, char **argv)
{
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-o"))
        {
            ++i;
            return argv[i];
        }
    }
    return "out";
}

char **split_by_line(char *str, int *num_out)
{
    int num_line = 1;
    int len = strlen(str);

    for (int i = 0; i < len; ++i)
    {
        if (str[i] == '\n')
        {
            ++num_line;
        }
    }
    *num_out = num_line;

    char **split_lines = NULL;
    int line = 0;
    int line_start = 0;

    for (int i = 0; i < len; ++i)
    {
        if (str[i] == '\n')
        {
            int line_len = i - line_start;
            char *line_str = calloc(line_len + 1, 1);
            if (!line_str)
            {
                return NULL;
            }
            memcpy(line_str, &str[line_start], line_len);
            split_lines = realloc(split_lines, (line + 1) * sizeof(char *));
            if (!split_lines)
            {
                return NULL;
            }
            split_lines[line++] = line_str;
            line_start = i + 1;
        }
    }

    if (line_start <= len)
    {
        int line_len = len - line_start;
        char *line_str = calloc(line_len + 1, 1);
        if (!line_str)
        {
            return NULL;
        }
        memcpy(line_str, &str[line_start], line_len);
        split_lines = realloc(split_lines, (line + 1) * sizeof(char *));
        if (!split_lines)
        {
            return NULL;
        }
        split_lines[line++] = line_str;
    }

    return split_lines;
}

bool starts_with(char *str, char *predicate)
{
    uint64_t pred_len = strlen(predicate);
    uint64_t str_len = strlen(str);
    if (pred_len > str_len)
    {
        return false;
    }
    for (int i = 0; i < pred_len; ++i)
    {
        if (str[i] != predicate[i])
        {
            return false;
        }
    }
    return true;
}

char **get_includes(char *src, int *num_include)
{
    int line_count = 0;
    char **lines = split_by_line(src, &line_count);
    char **includes = malloc(sizeof(char **));
    int num = 0;
    for (int i = 0; i < line_count; ++i)
    {
        if (starts_with(lines[i], "include"))
        {
            ++num;
            includes = realloc(includes, sizeof(char *) * num);
            includes[num - 1] = lines[i];
        }
        else
        {
            free(lines[i]);
        }
    }
    *num_include = num;
    return includes;
}
char *src_after_includes(char *src)
{
    int num_include = 0;
    char **includes = get_includes(src, &num_include);
    char *src_cpy = calloc(strlen(src) + 1, 1);
    strcpy(src_cpy, src);
    for (int i = 0; i < num_include; ++i)
    {
        char *chop_include = includes[i];
        chop_include += strlen("include");
        char *fp = calloc(strlen(chop_include) + 1, 1);
        int j = 0;
        while (*chop_include)
        {
            if (*chop_include == '"')
            {
                ++chop_include;
                while (*chop_include != '"' && *chop_include)
                {
                    fp[j] = *chop_include;
                    ++j;
                    ++chop_include;
                }
                break;
            }
            ++chop_include;
        }
        char *new_src = read_whole_file(fp);
        if (error_count())
        {
            put_errors();
        }
        new_src = src_after_includes(new_src);
        src_cpy = realloc(src_cpy, strlen(new_src) + strlen(src_cpy) + 1);
        strcat(src_cpy, "\n");
        strcat(src_cpy, new_src);
    }
    return src_cpy;
}

int main(int argc, char **argv)
{
    char *input_file_path = find_input_file(argc, argv);
    char *input_file = read_whole_file(input_file_path);
    if (error_count())
    {
        put_errors(); // will exit if errors exist.
    }
    input_file = src_after_includes(input_file);
    printf("Compiling file '%s'\n", input_file_path);
    if (logging)
    {
        printf("%s\n\n\n", input_file);
    }
    int num = 0;
    token_t *tokens = lex_all(input_file, &num);
    int i = 0;
    while (i < num)
    {
        token_t t = tokens[i];
        if (t.type == END_OF_FILE)
        {
            break;
        }
        if (logging)
        {
            printf("%s: %d\n", t.text, t.type);
        }
        ++i;
    }
    if (error_count())
    {
        put_errors(); // will exit if errors exist.
    }
    bytecode_gen_t gen;
    gen.tokens = tokens;
    gen.pos = 0;
    gen.num_tok = num;
    if (logging)
    {
        printf("\n\n");
    }

    instruction_t *instructions = malloc(sizeof(instruction_t));
    uint64_t num_ins = 0;
    while (1)
    {
        instruction_t ins = compile(&gen);
        if (ins.opcode == END)
        {
            break;
        }
        if (logging)
        {
            printf("%s: %s\n", opcode_to_string(ins.opcode), ins.num_arg > 0 ? ins.arguments[0].val : "no args");
        }
        ++num_ins;
        instructions = realloc(instructions, num_ins * sizeof(instruction_t));
        instructions[num_ins - 1] = ins;
    }
    if (error_count())
    {
        put_errors();
    }
    if (logging)
    {
        printf("\n\n");
    }
    compiler_t compiler;
    compiler.assembly = 0;
    compiler.instructions = instructions;
    compiler.num_instruction = num_ins;
    compiler.pos = 0;
    compiler.len = 0;
    compiler.num_strings = 0;
    compiler.string_table = 0;
    compiler.num_var = 0;
    compiler.var_table = 0;
    compile_asm(&compiler);
    if (error_count())
    {
        put_errors();
    }
    if (logging)
    {
        printf("%s\n", compiler.assembly);
    }
    char *output_file = get_output(argc, argv);
    char *output_asm = malloc(strlen(output_file) + 5);
    strcpy(output_asm, output_file);
    strcat(output_asm, ".asm");
    FILE *f = fopen(output_asm, "wb");
    fwrite(compiler.assembly, strlen(compiler.assembly), 1, f);
    fclose(f);
    char *output_o = malloc(strlen(output_file) + 3);
    strcpy(output_o, output_file);
    strcat(output_o, ".o");
    char cmd[1024];
    sprintf(cmd, "nasm -f elf64 %s -o %s", output_asm, output_o);
    system(cmd);
    char cmd1[1024];
    sprintf(cmd1, "ld %s -o %s -entry=main", output_o, output_file);
    system(cmd1);
    char cmd2[1024];
    sprintf(cmd2, "rm %s %s", output_asm, output_o);
    system(cmd2);
    printf("File '%s' compiled to '%s'.\n", input_file_path, output_file);
}
