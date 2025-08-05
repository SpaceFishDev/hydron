#ifndef COMPILER_H
#define COMPILER_H

#include "bytecode.h"

typedef struct
{
    char *assembly;
    uint64_t len;
    instruction_t *instructions;
    uint64_t num_instruction;
    uint64_t pos;
    char **string_table;
    uint64_t num_strings;
} compiler_t;

void add_asm(char *assembly, compiler_t *compiler);
void compile_asm(compiler_t *compiler);
uint64_t get_string(char *str, compiler_t *compiler);

#endif