#include "utils.h"
#include "error.h"
#include "lexer.h"
#include "bytecode.h"
#include "compiler.h"
#include <unistd.h>
#include <stdlib.h>

const int logging = 0;

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

int main(int argc, char **argv)
{
    char *input_file_path = find_input_file(argc, argv);
    char *input_file = read_whole_file(input_file_path);
    if (error_count())
    {
        put_errors(); // will exit if errors exist.
    }
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
