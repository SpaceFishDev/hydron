#include <stdio.h>
#include <string.h>
#include "bytecode.h"
#include "error.h"

#define current (gen->tokens[gen->pos])
#define next gen->pos++

char *opcode_to_string(opcode_type opcode)
{
    switch (opcode)
    {
    case END:
        return "end";
    case PUSH:
        return "push";
    case DROP:
        return "drop";
    case ADD:
        return "add";
    case SUB:
        return "sub";
    case MUL:
        return "mul";
    case DIV:
        return "div";
    case LABEL:
        return "label";
    case EXIT:
        return "exit";
    case DUP:
        return "dup";
    case LESS:
        return "less";
    case MORE:
        return "more";
    case EQ:
        return "eq";
    case BR:
        return "branch";
    case RETURN:
        return "return";
    case CALL:
        return "call";
    case GOTO:
        return "goto";
    case PRINT:
        return "print";
    case PRINTNUM:
        return "print_num";
    case MOD:
        return "mod";
    case FOPEN:
        return "fopen";
    case FREAD:
        return "fread";
    case FWRITE:
        return "fwrite";
    case FSEEK:
        return "fseek";
    case FTELL:
        return "ftell";
    case RESERVE:
        return "reserve";
    case SET_PTR:
        return "set_ptr";
    case GET_PTR:
        return "get_ptr";
    case SWAP:
        return "swap";
    case DECL:
        return "declare";
    case SET_VAR:
        return "set_var";
    case REF_VAR:
        return "ref_var";
    case SRESERVE:
        return "sreserve";
    case SET_BYTE:
        return "set_byte";
    case READC:
        return "readc";
    case FUNC:
        return "function";
    case BRK:
        return "brk";
    default:
        return "unknown";
    }
}

error_t unexpected_token(token_type expected, token_t offending_token)
{
    error_t err;
    sprintf(err.message, "Unexpected token `%s`, expected one of type `%d`", offending_token.text, expected);
    err.line = offending_token.line;
    err.column = offending_token.column;
    err.is_warning = 0;
    err.dont_show_line_col = 0;
    return err;
}

instruction_t compile(bytecode_gen_t *gen)
{
    if (current.type == END_OF_FILE || gen->pos >= gen->num_tok)
    {
        return INSTRUCTION(END, {}, 0);
    }
    if (current.type != WORD)
    {
        error_t err;
        sprintf(err.message, "Floating value of type `%d` with a value of `%s`", current.type, current.text);
        err.line = current.line;
        err.column = current.column;
        err.is_warning = 0;
        err.dont_show_line_col = 0;
        add_error(err);
        return INSTRUCTION(END, {}, 0);
    }
    if (!strcmp(current.text, "push"))
    {
        arg_t arg;
        next;
        arg.t = current.type == INTEGER ? INT : ((current.type == WORD) ? LABEL_NAME : STR);
        arg.val = current.text;
        next;
        return INSTRUCTION(PUSH, {arg}, 1);
    }
    if (!strcmp(current.text, "add"))
    {
        next;
        return INSTRUCTION(ADD, {}, 0);
    }
    if (!strcmp(current.text, "drop"))
    {
        next;
        return INSTRUCTION(DROP, {}, 0);
    }
    if (!strcmp(current.text, "sub"))
    {
        next;
        return INSTRUCTION(SUB, {}, 0);
    }
    if (!strcmp(current.text, "mul"))
    {
        next;
        return INSTRUCTION(MUL, {}, 0);
    }
    if (!strcmp(current.text, "div"))
    {
        next;
        return INSTRUCTION(DIV, {}, 0);
    }
    if (!strcmp(current.text, "label"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        char *name = current.text;
        arg_t arg;
        arg.t = LABEL_NAME;
        arg.val = name;
        next;
        return INSTRUCTION(LABEL, {arg}, 1);
    }
    if (!strcmp(current.text, "exit"))
    {
        next;
        return INSTRUCTION(EXIT, {}, 0);
    }
    if (!strcmp(current.text, "dup"))
    {
        next;
        return INSTRUCTION(DUP, {}, 0);
    }
    if (!strcmp(current.text, "less"))
    {
        next;
        return INSTRUCTION(LESS, {}, 0);
    }
    if (!strcmp(current.text, "more"))
    {
        next;
        return INSTRUCTION(MORE, {}, 0);
    }
    if (!strcmp(current.text, "eq"))
    {
        next;
        return INSTRUCTION(EQ, {}, 0);
    }
    if (!strcmp(current.text, "goto"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg0;
        arg0.t = LABEL_NAME;
        arg0.val = current.text;
        next;
        return INSTRUCTION(GOTO, {arg0}, 1);
    }
    if (!strcmp(current.text, "br"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg0;
        arg0.t = LABEL_NAME;
        arg0.val = current.text;
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg1;
        arg1.t = LABEL_NAME;
        arg1.val = current.text;
        next;
        instruction_t ins;
        ins.arguments[0] = arg0;
        ins.arguments[1] = arg1;
        ins.opcode = BR;
        ins.num_arg = 2;
        return ins;
    }
    if (!strcmp(current.text, "call"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg0;
        arg0.t = LABEL_NAME;
        arg0.val = current.text;
        next;
        instruction_t ins = INSTRUCTION(CALL, {arg0}, 1);
        return ins;
    }
    if (!strcmp(current.text, "return"))
    {
        next;
        return INSTRUCTION(RETURN, {}, 0);
    }
    if (!strcmp(current.text, "print"))
    {
        next;
        return INSTRUCTION(PRINT, {}, 0);
    }
    if (!strcmp(current.text, "printnum"))
    {
        next;
        return INSTRUCTION(PRINTNUM, {}, 0);
    }
    if (!strcmp(current.text, "mod"))
    {
        next;
        return INSTRUCTION(MOD, {}, 0);
    }
    if (!strcmp(current.text, "reserve"))
    {
        next;
        if (current.type != INTEGER)
        {
            error_t err = unexpected_token(INTEGER, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg;
        arg.t = INT;
        arg.val = current.text;
        next;
        return INSTRUCTION(RESERVE, {arg}, 1);
    }
    if (!strcmp(current.text, "get_ptr"))
    {
        next;
        return INSTRUCTION(GET_PTR, {}, 0);
    }
    if (!strcmp(current.text, "set_ptr"))
    {
        next;
        return INSTRUCTION(SET_PTR, {}, 0);
    }
    if (!strcmp(current.text, "swap"))
    {
        next;
        return INSTRUCTION(SWAP, {}, 0);
    }
    if (!strcmp(current.text, "decl"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg;
        arg.t = LABEL_NAME;
        arg.val = current.text;
        next;
        return INSTRUCTION(DECL, {arg}, 1);
    }
    if (!strcmp(current.text, "set_var"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg;
        arg.t = LABEL_NAME;
        arg.val = current.text;
        next;
        return INSTRUCTION(SET_VAR, {arg}, 1);
    }
    if (!strcmp(current.text, "ref_var"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg;
        arg.t = LABEL_NAME;
        arg.val = current.text;
        next;
        return INSTRUCTION(REF_VAR, {arg}, 1);
    }
    if (!strcmp(current.text, "sreserve"))
    {
        next;
        return INSTRUCTION(SRESERVE, {}, 0);
    }
    if (!strcmp(current.text, "fopen"))
    {
        next;
        return INSTRUCTION(FOPEN, {}, 0);
    }
    if (!strcmp(current.text, "fclose"))
    {
        next;
        return INSTRUCTION(FCLOSE, {}, 0);
    }
    if (!strcmp(current.text, "fread"))
    {
        next;
        return INSTRUCTION(FREAD, {}, 0);
    }
    if (!strcmp(current.text, "fseek"))
    {
        next;
        return INSTRUCTION(FSEEK, {}, 0);
    }
    if (!strcmp(current.text, "ftell"))
    {
        next;
        return INSTRUCTION(FTELL, {}, 0);
    }
    if (!strcmp(current.text, "putc"))
    {
        next;
        return INSTRUCTION(PUTC, {}, 0);
    }
    if (!strcmp(current.text, "get_byte"))
    {
        next;
        return INSTRUCTION(GET_BYTE, {}, 0);
    }
    if (!strcmp(current.text, "set_byte"))
    {
        next;
        return INSTRUCTION(SET_BYTE, {}, 0);
    }
    if (!strcmp(current.text, "readc"))
    {
        next;
        return INSTRUCTION(READC, {}, 0);
    }
    if (!strcmp(current.text, "func"))
    {
        next;
        if (current.type != WORD)
        {
            error_t err = unexpected_token(WORD, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        arg_t arg;
        arg.t = LABEL_NAME;
        arg.val = current.text;
        next;
        return INSTRUCTION(FUNC, {arg}, 1);
    }
    if (!strcmp(current.text, "include"))
    {
        next;
        if (current.type != STRING)
        {
            error_t err = unexpected_token(STRING, current);
            add_error(err);
            return INSTRUCTION(END, {}, 0);
        }
        next;
        return compile(gen);
    }
    if (!strcmp(current.text, "brk"))
    {
        next;
        return INSTRUCTION(BRK, {}, 0);
    }

    error_t err;
    sprintf(err.message, "Unknown instruction `%s`", current.text);
    err.line = current.line;
    err.column = current.column;
    err.is_warning = 0;
    err.dont_show_line_col = 0;
    add_error(err);
    return INSTRUCTION(END, {}, 0);
}