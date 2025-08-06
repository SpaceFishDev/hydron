#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>
#include "lexer.h"

typedef enum
{
    END,
    PUSH,
    DROP,
    ADD,
    SUB,
    MUL,
    DIV,
    LABEL,
    DUP,
    LESS,
    MORE,
    EQ,
    BR,
    GOTO,
    RETURN,
    CALL,
    PRINTNUM,
    PRINT,
    MOD,
    FOPEN,
    FREAD,
    FWRITE,
    FSEEK,
    FTELL,
    RESERVE,
    SET_PTR,
    GET_PTR,
    SWAP,
    DECL,
    SET_VAR,
    GET_VAR,
    EXIT,
} opcode_type;

typedef enum
{
    INT,
    STR,
    LABEL_NAME,
} arg_type;

typedef struct
{
    arg_type t;
    char *val;
} arg_t;

typedef struct
{
    opcode_type opcode;
    arg_t arguments[16];
    uint64_t num_arg;
} instruction_t;

typedef struct
{
    token_t *tokens;
    uint64_t pos;
    uint64_t num_tok;
} bytecode_gen_t;

instruction_t compile(bytecode_gen_t *gen);
#define INSTRUCTION(opcode, arguments, num_arg) \
    ((instruction_t){opcode, arguments, num_arg})

char *opcode_to_string(opcode_type opcode);
#endif