//=============================================================================
// src\compile.c
//-----------------------------------------------------------------------------
// Define as funções usadas para compilar código Burn.
//
// As funções definidas aqui incluem magias negras fortes e só devem ser
// alteradas em caso de necessidade real. Preze por sua vida, jovem mago.
//=============================================================================
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "vm.h"
#include "lexer.h"
#include "parser.h"
#include "errors.h"

#ifdef __DEBUG__
#include <windows.h> // PARA BENCHMARK, APAGAR DEPOIS!
#endif // __DEBUG__

//=============================================================================
//  Declaração
//-----------------------------------------------------------------------------
// As funções que são definidas nesse arquivo devem primeiro ser declaradas
// aqui
//=============================================================================
void          __error                       (unsigned int, const char*, const char*, ...);
shiro_binary  __binary_concat               (shiro_binary*, shiro_binary*);
void          __free_binary                 (shiro_binary*);
shiro_binary  __binary_concat_and_destroy   (shiro_binary*, shiro_binary*);
shiro_binary  __compile_statement           (const char*, unsigned int*);
//=============================================================================
//  Implementação
//-----------------------------------------------------------------------------
// Daqui pra baixo começam umas tretas fortes com ponteiros. Não se arrisque
// demais mexendo nisso.
//=============================================================================
//-----------------------------------------------------------------------------
// Lança uma mensagem de erro
//      errcode     : Nome do tipo de erro
//      message     : Mensagem de erro
//-----------------------------------------------------------------------------
void __error(unsigned int line, const char* errcode, const char* message, ...) {
    char* err = malloc(1024);
    sprintf(err, "%s on line %d: %s", errcode, line, message);

    va_list args;
    va_start(args, message);
    vfprintf(stderr, err, args);
    va_end(args);

    exit(1);
}
//-----------------------------------------------------------------------------
// Concatena dois códigos Shiro compilados
//      a   : Primeiro binário
//      b   : Segundo binário
//-----------------------------------------------------------------------------
shiro_binary __binary_concat(shiro_binary* a, shiro_binary* b) {

    shiro_binary r;
    r.size = a->size + b->size;

    if (a->size == 0) {
        r.nodes = b->nodes;
    } else if (b->size == 0) {
        r.nodes = a->nodes;
    } else {
        r.nodes = calloc(5, sizeof(shiro_node));
        memcpy(r.nodes, a->nodes, a->size * sizeof(shiro_node));
        memcpy(r.nodes + a->size * sizeof(shiro_node),
                b->nodes, b->size * sizeof(shiro_node));
    }

    return r;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por um código Shiro compilado
//      bin : Binário a ser liberado
//-----------------------------------------------------------------------------
void __free_binary(shiro_binary* bin) {
    unsigned int i;
    for (i = 0; i < bin->size; i++) {
        unsigned int j;
        for (j = 0; j < bin->nodes[i].n_args; j++)
            free(bin->nodes[i].args[j]);
        //free(bin->nodes[i].args);
    }
}
//-----------------------------------------------------------------------------
// Concatena dois códigos Shiro compilados e os limpa da memória
//      a   : Primeiro binário
//      b   : Segundo binário
//-----------------------------------------------------------------------------
shiro_binary __binary_concat_and_destroy(shiro_binary* a, shiro_binary* b) {
    shiro_binary r = __binary_concat(a, b);

    __free_binary(a);
    __free_binary(b);

    return r;
}
//-----------------------------------------------------------------------------
// Compila uma expressão
//      statement   : Expressão
//-----------------------------------------------------------------------------
shiro_binary __compile_statement(const char* statement, unsigned int* line) {
    shiro_binary binary;
    binary.size = 0;

    if (*statement == 0)
        return binary;

    unsigned int start_line = *line;

    // Obtém a enésima token da sentença
    const char* __token(int n) {
        *line = start_line;

        int i, j;

        for (i = j = 0; j < n && (statement[i] + statement[i + 1]) != 0; i++, j++) {

            if (strcmp(statement + i, MARK_OBLOCK) == 0)
                while (strcmp(statement + i, MARK_CBLOCK) != 0)
                    i++;
            else if (strcmp(statement + i, MARK_OEXPR) == 0)
                while (strcmp(statement + i, MARK_CEXPR) != 0)
                    i++;

            while (statement[i] != 0) {
                if (statement[i] == *MARK_EOL)
                    (*line)++;
                i++;
            }
        }

        if (i > 0 && (statement[i] + statement[i - 1]) == 0)
            return NULL;
        return statement + i;
    }

    // Obtém uma expressão a partir de um token '('
    const char* __expr(const char* token) {

        unsigned int
            e_sz = DF_TOKEN_SIZE,
            e = 0;
        char* expression = malloc(e_sz);
        memset(expression, 0, e_sz);

        unsigned int p_stack = 1;

        while (*token != 0)
            token++;
        token++;

        while (p_stack > 0) {
            if (strcmp(token, MARK_OEXPR) == 0)
                p_stack++;
            else if (strcmp(token, MARK_CEXPR) == 0)
                p_stack--;

            if (p_stack > 0) {
                expression = push_token(expression, &e_sz, &e, token);
                while (*token != 0) {
                    token++;
                }
                token++;
            }
        }

        return expression;
    }

    const char* token;
    token = __token(0);

    switch (get_token_type(token)) {
        case TK_KEYWORD:
            /*
                #define KW_COND         "if"
                #define KW_ELSE         "else"
                #define KW_PRIVATE      "private"
                #define KW_PROTECTED    "protected"
                #define KW_PUBLIC       "public"
                #define KW_SWITCH       "switch"
                #define KW_CASE         "case"
                #define KW_FOR          "for"
                #define KW_WHILE        "while"
                #define KW_LOOP         "loop"
                #define KW_BREAK        "break"
                #define KW_NEXT         "continue"
                #define KW_DELETE       "delete"
                #define KW_FUNC         "fn"
                #define KW_PROC         "proc"
                #define KW_VAR          "var"
                #define KW_CONST        "const"
                #define KW_CLASS        "class"
            */
            if (strcmp(token, KW_COND) == 0)
            {
                token = __token(1);
                if (token != NULL && strcmp(token, MARK_OEXPR) == 0) {

                    const char* expression = __expr(token);
                    token = __token(2);

                    if (token != NULL && strcmp(token, MARK_OBLOCK) == 0) {

                        token = __token(3);
                        if (token != NULL && strcmp(token, KW_ELSE) == 0) {

                        } else if (token != NULL && strcmp(token, MARK_EOS) == 0) {
                            shiro_binary b_expr = __compile_statement(expression, line);

                            binary.size = 1;
                            binary.nodes = calloc(binary.size, sizeof(shiro_node));
                            binary.nodes[0] = (shiro_node){ COND, NULL, 0 };

                            binary = __binary_concat_and_destroy(&b_expr, &binary);
                        } else if (token != NULL) {
                            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token);
                            return binary;
                        } else {
                            __error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting '%s'", MARK_EOS);
                            return binary;
                        }
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token, MARK_OBLOCK);
                        return binary;
                    }
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token, MARK_OEXPR);
                    return binary;
                }
            }

            break;
        case TK_NAME:
            break;
        case TK_MARK:
            break;
        case TK_CONST:
        {
            const char* value = token;

            token = __token(1);
            if (token != NULL) {

            } else {
                binary.size = 1;
                binary.nodes = calloc(1, sizeof(shiro_node));

                shiro_value s_value;

                if (value[0] == *MARK_STR1 || value[0] == *MARK_STR2) {
                    s_value.type = s_tString;
                    s_value.fields = calloc(2, sizeof(union shiro_field));

                    shiro_string v = malloc(strlen(value) - 1);
                    memset(v, 0, strlen(value) - 1);
                    memcpy(v, value + 1, strlen(value) - 1);

                    s_value.fields[0].str = (shiro_string)v;
                    s_value.fields[1].i   = (shiro_fixnum)strlen(s_value.fields[0].str);
                    s_value.n_fields = 2;
                } else {
                    s_value.type = s_tInt;
                    s_value.fields = calloc(1, sizeof(union shiro_field));

                    char* end;
                    s_value.fields[0].i = (shiro_fixnum)strtol(value, &end, 10);
                    s_value.n_fields = 1;
                }

                shiro_value* args = calloc(1, sizeof(shiro_value));
                args[0] = s_value;

                binary.nodes[0] = (shiro_node){PUSH, (void**)&args, 1};
            }
            break;
        }
        default:
            __error(*line, ERR_SYNTAX_ERROR, "Unexpected token '%s'", token);
            break;
    }

    return binary;
}
//-----------------------------------------------------------------------------
// Compila o código passado
//      code    : Código
//-----------------------------------------------------------------------------
shiro_binary shiro_compile(const char* code) {
    char* tokens = malloc(strlen(code) * 2 + 1);
    memset(tokens, 0, strlen(code) * 2 + 1);
    shiro_tokenize(code, tokens);

    unsigned int i = 0, t_sz = DF_TOKEN_SIZE, t = 0;
    char* token = malloc(t_sz);
    memset(token, 0, t_sz);

    unsigned int s_sz = strlen(code) / 2 + 1, s = 0;
    char* statement = malloc(s_sz);
    memset(statement, 0, s_sz);

    shiro_binary binary;
    binary.size = 0;

    unsigned int line_compiled = 1,
                 line = 1,
                 stack = 0,
                 p_stack = 0;

    void __process_token() {
        if (*token == 0)
            return;

        if (strcmp(token, MARK_EOL) == 0) {
            line++;
        } else if (strcmp(token, MARK_OBLOCK) == 0)
            if (p_stack != 0) {
                __error(line, ERR_SYNTAX_ERROR,
                            "Unexpected '%s', expecting '%s'",
                            MARK_OBLOCK, MARK_CEXPR);
                return;
            } else
                stack++;
        else if (strcmp(token, MARK_CBLOCK) == 0) {
            if (stack > 0)
                stack--;
            else {
                __error(line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CBLOCK);
                return;
            }
        } else if (strcmp(token, MARK_OEXPR) == 0) {
            p_stack++;
        } else if (strcmp(token, MARK_CEXPR) == 0) {
            if (p_stack > 0)
                p_stack--;
            else {
                __error(line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CEXPR);
                return;
            }
        }
        statement = push_token(statement, &s_sz, &s, token);

        if (strcmp(token, MARK_EOS) == 0 && stack == 0 && p_stack == 0) {
            shiro_binary other = __compile_statement(statement, &line_compiled);

            if (other.size > 0) {
                if (binary.size > 0)
                    binary = __binary_concat_and_destroy(&binary, &other);
                else
                    binary = other;
            }

            free(statement);
            statement = malloc(s_sz);
            memset(statement, 0, s_sz);
            s = 0;
        }
    }

    for (; (tokens[i] + tokens[i + 1]) != 0; i++) {
        if (tokens[i] == 0) {
            __process_token();
            token = clear_token(token, &t_sz, &t);
        } else
            token = append_to_string(token, &t_sz, &t, tokens[i]);
    }
    free(tokens);

    __process_token();

    free(token);

    if (stack > 0)
        __error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CBLOCK);
    else if (p_stack > 0)
        __error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CEXPR);

    shiro_binary other = __compile_statement(statement, &line_compiled);

    if (other.size > 0) {
        if (binary.size > 0)
            binary = __binary_concat_and_destroy(&binary, &other);
        else
            binary = other;
    }

    free(statement);

    return binary;
}
//-----------------------------------------------------------------------------
// Ponto de entrada para teste
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
    static const char* code = "if (1) { print('Hello World'); };";

    double get_time() {
        LARGE_INTEGER t, f;
        QueryPerformanceCounter(&t);
        QueryPerformanceFrequency(&f);
        return (double)t.QuadPart/(double)f.QuadPart;
    }

    double t0 = get_time();

    for (;;) {
        shiro_binary bin = shiro_compile(code);
        printf("%d node(s) compiled\n", bin.size);
    }
    printf("\n\n%f ms\n", (get_time() - t0) * 1000);

    return 0;
}
