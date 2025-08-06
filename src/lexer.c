#include "lexer.h"
#include "error.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define current (lexer->pos < strlen(lexer->src) ? lexer->src[lexer->pos] : (char)0)
#define next                                                               \
    lexer->column++;                                                       \
    if (lexer->pos < strlen(lexer->src) && lexer->src[lexer->pos] == '\n') \
    {                                                                      \
        lexer->line++;                                                     \
        lexer->column = 0;                                                 \
    }                                                                      \
    lexer->pos++
#define current_is_letter \
    ((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z'))
#define current_is_num \
    (current >= '0' && current <= '9')
token_t lex(lexer_t *lexer)
{
    if (current == '#' || current == ';')
    {
        char c = current;
        next;
        while (current && current != '\n' && current != c)
        {
            next;
        }
        return lex(lexer);
    }
    if (current_is_letter)
    {
        uint64_t p = lexer->pos;
        uint64_t c = lexer->column;
        while (current_is_letter || current == '_' || current_is_num)
        {
            next;
        }
        uint64_t len = lexer->pos - p;
        lexer->pos = p;
        lexer->column = c;
        char *buffer = calloc(len + 1, 1);
        char *ptr = buffer;
        while (current_is_letter || current == '_' || current_is_num)
        {
            *ptr = current;
            next;
            ++ptr;
        }
        return TOKEN(WORD, buffer, lexer->line, c);
    }
    bool is_neg = false;
    if (current == '-')
    {
        next;
        if (current_is_num)
        {
            is_neg = true;
        }
    }
    if (current_is_num || is_neg)
    {
        uint64_t p = lexer->pos;
        uint64_t c = lexer->column;
        bool neg_encountered = false;
        while (current_is_num || (current == '-' && !neg_encountered))
        {
            if (current == '-')
            {
                neg_encountered = true;
            }
            next;
        }
        uint64_t len = lexer->pos - p;
        lexer->pos = p;
        lexer->column = c;
        char *buffer = calloc(len + 1, 1);
        char *ptr = buffer;
        if (is_neg)
        {
            *ptr = '-';
            ++ptr;
        }
        int i = 0;
        while (i < len)
        {
            *ptr = current;
            ++ptr;
            ++i;
            next;
        }
        return TOKEN(INTEGER, buffer, lexer->line, c);
    }
    switch (current)
    {
    case (char)0:
    {
        return TOKEN(END_OF_FILE, "\0", lexer->line, lexer->column);
    }
    case '"':
    {
        next;
        uint64_t p = lexer->pos;
        uint64_t c = lexer->column;
        while (current != '"')
        {
            if (current == '\n' || current == 0)
            {
                error_t err;
                sprintf(err.message, "String is not terminated.");
                err.line = lexer->line;
                err.column = c;
                err.is_warning = 0;
                err.dont_show_line_col = 0;
                add_error(err);
                return TOKEN(END_OF_FILE, "\0", lexer->line, c);
            }
            next;
        }
        uint64_t len = lexer->pos - p;
        lexer->pos = p;
        lexer->column = c;
        char *buffer = calloc(len + 1, 1);
        char *ptr = buffer;
        while (current != '"')
        {
            *ptr = current;
            ptr++;
            next;
        }
        next;
        return TOKEN(STRING, buffer, lexer->line, c);
    }
    case ' ':
    case '\t':
    case '\n':
    {
        next;
        return lex(lexer);
    }
    default:
    {
        error_t err;
        sprintf(err.message, "Unexpected token `%c`", current);
        err.line = lexer->line;
        err.column = lexer->column;
        err.is_warning = 0;
        err.dont_show_line_col = 0;
        add_error(err);
        return TOKEN(END_OF_FILE, "\0", lexer->line, lexer->column);
    }
    }
}