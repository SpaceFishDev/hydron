#include "compiler.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void add_asm(char *assembly, compiler_t *compiler)
{
    if (!assembly)
    {
        return;
    }
    if (!compiler->assembly)
    {
        compiler->assembly = calloc(strlen(assembly) + 1, 1);
        strcpy(compiler->assembly, assembly);
        compiler->len = strlen(assembly);
        return;
    }
    compiler->len += strlen(assembly);
    char *new_buf = calloc(compiler->len + 1, 1);
    strcpy(new_buf, compiler->assembly);
    char *old = compiler->assembly;
    compiler->assembly = new_buf;
    free(old);
    strcat(compiler->assembly, assembly);
}

#define current (compiler->instructions[compiler->pos])

int iota = 0;
static char static_buffer[1024 * 8];
char *compile_print(compiler_t *compiler)
{
    memset(static_buffer, 0, 1024 * 8);
    int l0 = iota;
    ++iota;
    int l1 = iota;
    ++iota;
    sprintf(static_buffer, "pop rdi\nmov r8, rdi\nxor r9d, r9d\n.l%d:\ncmp byte [r8+r9],0\nje .l%d\ninc r9\njmp .l%d\n.l%d:\nmov r10d, 1\nmov rax, r10\nmov rdi, r10\nmov rsi, r8\nmov rdx, r9\nsyscall\n",
            l0, l1, l0, l1);
    char *to_add = calloc(strlen(static_buffer) + 1, 1);
    strcpy(to_add, static_buffer);
    compiler->pos++;
    return to_add;
}

char *compile_print_num(compiler_t *compiler)
{
    memset(static_buffer, 0, 1024 * 8);
    int l0 = iota;
    ++iota;
    int l1 = iota;
    ++iota;
    int l2 = iota;
    ++iota;
    int l3 = iota;
    ++iota;

    sprintf(static_buffer, "pop rdi\nmov rbx,0\nlea rcx, [rsp + 128]\nsub rsp, 136\nmov rsi, 0\ncmp rdi, 0\njne .l%d\nmov byte [rcx],'0'\nadd rbx, 1\ndec rcx\njmp .l%d\n.l%d:\ncmp rdi, 0\njg .l%d\nmov rsi, 1\ninc rbx\nneg rdi\n.l%d:\nmov rax, rdi\ncqo\nmov r9,10\nidiv r9\nmov r10,rdx\nadd r10, '0'\ndec rcx\nmov byte [rcx], r10b\ninc rbx\nmov rdi, rax\ncmp rdi, 0\njg .l%d\n.l%d:\ncmp rsi, 1\njne .l%d\ndec rcx\nmov byte [rcx], '-'\n.l%d:\nmov rax, 1\nmov rdi, 1\nmov rsi, rcx\nmov rdx, rbx\nsyscall\nadd rsp, 136\n",
            l0, l2, l0, l1, l1, l1, l2, l3, l3);

    char *to_copy = calloc(strlen(static_buffer) + 1, 1);
    strcpy(to_copy, static_buffer);
    compiler->pos++;
    return to_copy;
}

char *compile_ins(compiler_t *compiler)
{
    switch (current.opcode)
    {
    case FUNC:
    {
        char buffer[1024];
        sprintf(buffer, "%s:\npush rbp\nmov rbp, rsp\n", current.arguments[0].val);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        compiler->pos++;
        return to_add;
    }
    break;
    case EQ:
    {
        compiler->pos++;
        char buffer[1024];
        sprintf(buffer, "pop rcx\npop rax\ncmp rax, rcx\nsete al\nmovzx rax, al\npush rax\n", iota, iota);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        return to_add;
    }
    case MORE:
    {
        compiler->pos++;
        char buffer[1024];
        sprintf(buffer, "pop rcx\npop rax\ncmp rax, rcx\nmov rbx, 1\nmov rdx, 0\ncmovg rdx, rbx\npush rdx\n", iota, iota);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        return to_add;
    }
    case LESS:
    {
        compiler->pos++;
        char buffer[1024];
        sprintf(buffer, "pop rcx\npop rax\ncmp rax, rcx\nmov rbx, 1\nmov rdx, 0\ncmovl rdx, rbx\npush rdx\n", iota, iota);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        return to_add;
    }
    case BR:
    {
        char buffer[1024];
        sprintf(buffer, "pop rax\nlea rbx, [%s]\nlea rcx, [%s]\ncmp rax, 0\ncmovz rbx,rcx\njmp rbx\n", current.arguments[0].val, current.arguments[1].val);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        compiler->pos++;
        return to_add;
    }
    case PUSH:
    {
        if (current.arguments[0].t == STR)
        {
            uint64_t str_in_table = get_string(current.arguments[0].val, compiler);
            char buffer[1024];
            sprintf(buffer, "push string%ld\n", str_in_table);
            char *to_add = calloc(strlen(buffer) + 1, 1);
            strcpy(to_add, buffer);
            compiler->pos++;
            return to_add;
        }
        else if (current.arguments[0].t == INT)
        {
            char buffer[1024];
            sprintf(buffer, "push %s\n", current.arguments[0].val);
            char *to_add = calloc(strlen(buffer) + 1, 1);
            strcpy(to_add, buffer);
            compiler->pos++;
            return to_add;
        }
        else
        {
            char buffer[1024];
            sprintf(buffer, "lea rbx, [var_%s]\nmov rax, [rbx]\npush rax\n", current.arguments[0].val);
            char *to_add = calloc(strlen(buffer) + 1, 1);
            strcpy(to_add, buffer);
            compiler->pos++;
            return to_add;
        }
        return "";
    }
    case LABEL:
    {
        char buffer[1024];
        sprintf(buffer, "%s:\n", current.arguments[0].val);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        compiler->pos++;
        return to_add;
    }
    case ADD:
    {
        compiler->pos++;
        return "pop qword rax\npop qword rbx\nadd rax, rbx\npush rax\n";
    }
    case SUB:
    {
        compiler->pos++;
        return "pop qword rbx\npop qword rax\nsub rax, rbx\npush rax\n";
    }
    case MUL:
    {
        compiler->pos++;
        return "pop qword rax\npop qword rbx\nmul rbx\npush rax\n";
    }
    case DIV:
    {
        compiler->pos++;
        return "pop qword rbx\npop qword rax\ncqo\nidiv rbx\npush rax\n";
    }
    case CALL:
    {
        char buffer[1024];
        sprintf(buffer, "call %s\npush rax\n", current.arguments[0].val);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        compiler->pos++;
        return to_add;
    }
    case GOTO:
    {
        char buffer[1024];
        sprintf(buffer, "jmp %s\n", current.arguments[0].val);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        compiler->pos++;
        return to_add;
    }
    case EXIT:
    {
        compiler->pos++;
        return "pop qword rdi\nmov rax, 60\nsyscall\n";
    }
    case DUP:
    {
        compiler->pos++;
        return "pop rax\npush rax\npush rax\n";
    }
    case PRINTNUM:
    {
        char *pn = compile_print_num(compiler);
        return pn;
    }
    case PRINT:
    {
        char *p = compile_print(compiler);
        return p;
    }
    case DROP:
    {
        compiler->pos++;
        return "pop rax\n";
    }
    case MOD:
    {
        compiler->pos++;
        return "pop rbx\npop rax\nxor rdx, rdx\ndiv rbx\npush rdx\n";
    }
    case SWAP:
    {
        compiler->pos++;
        return "pop rax\npop rbx\npush rax\npush rbx\n";
    }
    case SRESERVE:
    {
        compiler->pos++;
        return "pop rax\nsub rsp,rax\nmov rcx,rax\nxor rdi,rdi\nmov rdi,rsp\nxor rax,rax\nrep stosb\nmov rbx,rsp\nadd rbx,rcx\npush rbx\n";
    }
    case RESERVE:
    {
        char *num = current.arguments[0].val;
        compiler->pos++;
        char buffer[1024];
        sprintf(buffer, "sub rsp, %s\nlea rax, [rsp + %s]\npush rax\n", num, num);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        return to_add;
    }
    case PUTC:
    {
        compiler->pos++;
        return "pop rax\nlea rsi, [cbuf]\nmov [rsi], al\nmov rax, 1\nmov rdi, 1\nmov rdx, 1\nsyscall\n";
    }
    case GET_PTR:
    {
        compiler->pos++;
        return "pop rax\nmov rbx, [rax]\npush rbx\n";
    }
    case SET_PTR:
    {
        compiler->pos++;
        return "pop rax\npop rbx\nmov [rbx], rax\n";
    }
    case DECL:
    {
        compiler->pos++;
        return "";
    }
    case SET_VAR:
    {
        char *var = current.arguments[0].val;
        compiler->pos++;
        char buffer[1024];
        sprintf(buffer, "pop rax\nlea rbx, [var_%s]\nmov [rbx], rax\n", var);
        char *to_add = calloc(strlen(buffer) + 1, 1);
        strcpy(to_add, buffer);
        return to_add;
    }
    case REF_VAR:
    {
        char *var = current.arguments[0].val;
        compiler->pos++;
        char buffer[1024];
        sprintf(buffer, "lea rbx, [var_%s]\npush rbx\n", var);
    }
    case FOPEN:
    {
        compiler->pos++;
        return "mov rax, 0x02\npop rdx\npop rsi\npop rdi\nsyscall\npush rax\n";
    }
    case FCLOSE:
    {
        compiler->pos++;
        return "mov rax, 0x03\npop rdi\nsyscall\n";
    }
    case FREAD:
    {
        compiler->pos++;
        return "mov rax, 0x00\npop rdx\npop rsi\npop rdi\nsyscall\npush rax\n";
    }
    case FWRITE:
    {
        compiler->pos++;
        return "mov rax, 0x01\npop rdx\npop rsi\npop rdi\nsyscall\npush rax\n";
    }
    case FSEEK:
    {
        compiler->pos++;
        return "mov rax, 0x08\npop rdx\npop rsi\npop rdi\nsyscall\npush rax\n";
    }
    case FTELL:
    {
        compiler->pos++;
        return "pop rdi\nxor rsi, rsi\nmov rdx,1\nmov rax, 0x08\nsyscall";
    }
    case RETURN:
    {
        compiler->pos++;
        return "pop rax\nmov rsp, rbp\npop rbp\nret\n";
    }
    case SET_BYTE:
    {
        compiler->pos++;
        return "pop rbx\npop rcx\nmovzx rax, bl\nmov byte [rcx], al\n";
    }
    case GET_BYTE:
    {
        compiler->pos++;
        return "pop rbx\nmovzx rax, byte [rbx]\npush rax\n";
    }
    case READC:
    {
        compiler->pos++;
        return "sub rsp, 8\nmov rax, 0\nmov rdi, 0\nlea rsi, [rsp]\nmov rdx, 1\nsyscall\n";
    }
    default:
    {
        error_t err;
        sprintf(err.message, "Could not compile `%s` instruction.\n", opcode_to_string(current.opcode));
        err.dont_show_line_col = 1;
        err.is_warning = 0;
        add_error(err);
        compiler->pos++;
    }
    }
    return "";
}

void add_string(const char *str, compiler_t *compiler)
{
    char **new_table = realloc(compiler->string_table, sizeof(char *) * (compiler->num_strings + 1));
    if (!new_table)
    {
        return;
    }
    compiler->string_table = new_table;

    char *stored = strdup(str);
    compiler->string_table[compiler->num_strings] = stored;
    compiler->num_strings++;

    size_t cap = strlen(str) * 4 + 32;
    char *out = malloc(cap);
    size_t out_i = 0;

    out[out_i++] = '"';

    for (char *p = str; *p; ++p)
    {
        if (*p == '\\')
        {
            ++p;
            if (!*p)
            {
                break;
            }

            switch (*p)
            {
            case 'n':
            {
                if (out_i + 8 >= cap)
                {
                    cap *= 2;
                    out = realloc(out, cap);
                    if (!out)
                    {
                        return;
                    }
                }
                out[out_i++] = '"';
                out[out_i++] = ',';
                out[out_i++] = '1';
                out[out_i++] = '0';
                out[out_i++] = ',';
                out[out_i++] = '"';
            }
            break;
            case 'r':
            {
                if (out_i + 8 >= cap)
                {
                    cap *= 2;
                    out = realloc(out, cap);
                    if (!out)
                    {
                        return;
                    }
                }
                out[out_i++] = '"';
                out[out_i++] = ',';
                out[out_i++] = '1';
                out[out_i++] = '3';
                out[out_i++] = ',';
                out[out_i++] = '"';
            }
            break;
            case 't':
            {
                if (out_i + 2 >= cap)
                {
                    cap *= 2;
                    out = realloc(out, cap);
                    if (!out)
                    {
                        return;
                    }
                }
                out[out_i++] = '\t';
            }
            break;
            case '\\':
            {
                if (out_i + 1 >= cap)
                {
                    cap *= 2;
                    out = realloc(out, cap);
                    if (!out)
                    {
                        return;
                    }
                }
                out[out_i++] = '\\';
            }
            break;
            case '"':
            {
                if (out_i + 1 >= cap)
                {
                    cap *= 2;
                    out = realloc(out, cap);
                    if (!out)
                    {
                        return;
                    }
                }
                out[out_i++] = '"';
            }
            break;
            default:
            {
                if (out_i + 1 >= cap)
                {
                    cap *= 2;
                    out = realloc(out, cap);
                    if (!out)
                    {
                        return;
                    }
                }
                out[out_i++] = *p;
            }
            break;
            }
        }
        else
        {
            if (out_i + 1 >= cap)
            {
                cap *= 2;
                out = realloc(out, cap);
                if (!out)
                {
                    return;
                }
            }
            out[out_i++] = *p;
        }
    }

    if (out_i + 2 >= cap)
    {
        cap *= 2;
        out = realloc(out, cap);
        if (!out)
        {
            return;
        }
    }
    out[out_i++] = '"';
    out[out_i] = '\0';

    int idx = (int)(compiler->num_strings - 1);
    size_t need = snprintf(NULL, 0, "string%d:\n db %s,0\n", idx, out);
    char *to_add = malloc(need + 1);
    if (!to_add)
    {
        free(out);
        return;
    }
    snprintf(to_add, need + 1, "string%d:\n db %s,0\n", idx, out);

    add_asm(to_add, compiler);

    free(to_add);
    free(out);
}

void add_var(char *name, compiler_t *compiler)
{
    char **new_table = realloc(compiler->var_table, sizeof(char *) * (compiler->num_var + 1));
    if (!new_table)
    {
        return;
    }
    compiler->var_table = new_table;
    char *stored = strdup(name);
    compiler->var_table[compiler->num_var] = stored;
    ++compiler->num_var;
    char buf[1024];
    sprintf(buf, "var_%s: dq 0\n");
    add_asm(buf, compiler);
}

void create_var_table(compiler_t *compiler)
{
    for (int i = 0; i < compiler->num_instruction; ++i)
    {
        if (compiler->instructions[i].opcode == DECL)
        {
            add_var(compiler->instructions[i].arguments[0].val, compiler);
        }
    }
}

void create_string_table(compiler_t *compiler)
{
    add_asm("bits 64\nglobal main\nsection .data\ncbuf: dq 0\n", compiler);
    for (int i = 0; i < compiler->num_instruction; ++i)
    {
        if (compiler->instructions[i].opcode == PUSH)
        {
            if (compiler->instructions[i].arguments[0].t == STR)
            {
                add_string(compiler->instructions[i].arguments[0].val, compiler);
            }
        }
    }
    create_var_table(compiler);
    add_asm("section .text\n", compiler);
}

uint64_t get_string(char *str, compiler_t *compiler)
{
    for (int i = 0; i < (int)compiler->num_strings; ++i)
    {
        if (!strcmp(str, compiler->string_table[i]))
        {
            return i;
        }
    }
    return (uint64_t)-1;
}

void compile_asm(compiler_t *compiler)
{
    create_string_table(compiler);
    while (compiler->pos < compiler->num_instruction)
    {
        char str[128];
        char *op = opcode_to_string(current.opcode);
        sprintf(str, "; instruction: %s\n", op);
        add_asm(str, compiler);
        add_asm(compile_ins(compiler), compiler);
        memset(str, 0, 128);
        sprintf(str, "; end of instruction: %s\n", op);
        add_asm(str, compiler);
    }
}