#ifndef LEXER_H
#define LEXER_H
#include <stdint.h>

typedef enum
{
    WORD,
    INTEGER,
    STRING,
    END_OF_FILE,
} token_type;

typedef struct
{
    token_type type;
    char *text;
    uint64_t line, column;
} token_t;

typedef struct
{
    char *src;
    uint64_t pos;
    uint64_t line, column;
} lexer_t;

token_t lex(lexer_t *lexer);
#define TOKEN(type, text, line, col) ((token_t){type, text, line, col})

#endif
