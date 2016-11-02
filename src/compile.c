//=============================================================================
// src\compile.c
//-----------------------------------------------------------------------------
// Define as fun��es usadas para compilar c�digo Shiro
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
//-----------------------------------------------------------------------------
// Lan�a uma mensagem de erro e aborta a execu��o do programa
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
// Obt�m uma express�o a partir de um token '('
//      stmt    : Ponteiro para o shiro_statement onde est� o token '('
//      offset  : �ndice do token no shiro_statement
//-----------------------------------------------------------------------------
shiro_statement* __expression(
    const shiro_statement* stmt,
    shiro_uint offset
) {
    shiro_statement* expression = new_statement(1);

    shiro_uint p_stack = 1, line = 0;
    shiro_token* token = get_token(stmt, offset, &line);

    shiro_uint i = 0;
    while (stmt->tokens[i] != token)
        i++;

    token = stmt->tokens[++i];

    while (p_stack > 0) {
        if (strcmp(token->value, MARK_OEXPR) == 0)
            p_stack++;
        else if (strcmp(token->value, MARK_CEXPR) == 0)
            p_stack--;

        if (p_stack > 0)
            push_token(expression, token);

        token = stmt->tokens[++i];
    }

    return expression;
}
//-----------------------------------------------------------------------------
// Obt�m um bloco de c�digo a partir de um token '{'
//      stmt    : Ponteiro para o shiro_statement onde est� o token '{'
//      offset  : �ndice do token no shiro_statement
//-----------------------------------------------------------------------------
shiro_statement* __block(
    const shiro_statement* stmt,
    shiro_uint offset
) {
    shiro_statement* expression = new_statement(1);

    shiro_uint stack = 1, line = 0;
    shiro_token* token = get_token(stmt, offset, &line);

    shiro_uint i = 0;
    while (stmt->tokens[i] != token)
        i++;

    token = stmt->tokens[++i];

    while (stack > 0) {
        if (strcmp(token->value, MARK_OBLOCK) == 0)
            stack++;
        else if (strcmp(token->value, MARK_CBLOCK) == 0)
            stack--;

        if (stack > 0)
            push_token(expression, token);

        token = stmt->tokens[++i];
    }

    return expression;
}
//-----------------------------------------------------------------------------
// Compila uma express�o
//      statement   : Express�o
//-----------------------------------------------------------------------------
shiro_binary* __compile_statement(
    const shiro_statement* statement,
    shiro_uint* line
) {
    shiro_binary* binary = new_binary();

    if (statement == SHIRO_NIL || statement->used == 0)
        return binary;

    const shiro_token* token;
    token = get_token(statement, 0, line);

    switch (get_token_type(token)) {
        case s_tkKeyword:
        {
            if (strcmp(token->value, KW_COND) == 0) {
                token = get_token(statement, 1, line);
                if (token != SHIRO_NIL && strcmp(token->value, MARK_OEXPR) == 0) {
                    shiro_statement* expression = __expression(statement, 1);
                    token = get_token(statement, 2, line);
                    if (token != SHIRO_NIL && strcmp(token->value, MARK_OBLOCK) == 0) {
                        shiro_statement* block = __block(statement, 2);
                        token = get_token(statement, 3, line);
                        if (token != SHIRO_NIL && strcmp(token->value, KW_ELSE) == 0) {

                        } else if (token == SHIRO_NIL || strcmp(token->value, MARK_EOS) == 0) {
                            shiro_binary* b_expr = __compile_statement(expression, line);
                            free_statement(expression);

                            shiro_binary* b_block = __compile_statement(block, line);
                            free_statement(block);

                            shiro_node* cond = new_node(COND, 0);
                            push_node(binary, cond);
                            free_node(cond);

                            shiro_node* jmp = new_node(JUMP, 1, new_shiro_fixnum(b_block->used));
                            push_node(binary, jmp);
                            free_node(jmp);

                            binary = concat_and_free_binary(b_expr, binary);
                            binary = concat_and_free_binary(binary, b_block);
                        } else {
                            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                            return binary;
                        }
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                        return binary;
                    }
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OEXPR);
                    return binary;
                }
            }

            break;
        }
        case s_tkName:
        {
            shiro_string name = calloc(token->used, sizeof(shiro_character));
            memcpy(name, token->value, token->used * sizeof(shiro_character));

            token = get_token(statement, 1, line);

            if (token == SHIRO_NIL || strcmp(token->value, MARK_EOS) == 0) {
                shiro_node* node = new_node(PUSH_BY_NAME, 1, new_shiro_string(name));
                push_node(binary, node);
                free_node(node);
            } else if (strcmp(token->value, MARK_PROP) == 0) {
                token = get_token(statement, 2, line);

                if (get_token_type(token) == s_tkName) {
                    shiro_statement* rest = malloc(sizeof(shiro_statement));
                    memcpy(rest, statement, sizeof(shiro_statement));
                    rest->tokens += sizeof(shiro_token*) * 2;
                    rest->allocated -= 2;
                    rest->used -= 2;

                    shiro_binary* b_rest = __compile_statement(rest, line);

                    free(rest);

                    shiro_node* node = new_node(PUSH_BY_NAME, 1, new_shiro_string(name));
                    push_node(binary, node);
                    free_node(node);

                    binary = concat_and_free_binary(binary, b_rest);
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting NAME", token->value);
                    return binary;
                }

            } else if (strcmp(token->value, MARK_OEXPR) == 0) {
                shiro_statement* expression = __expression(statement, 1);
                {
                    shiro_statement* arg = new_statement(1);
                    shiro_token* tk;
                    int i;
                    for (
                         i = 0, tk = get_token(expression, i, line);
                         tk != SHIRO_NIL;
                         tk = get_token(expression, ++i, line)
                    ) {
                        if (strcmp(tk->value, MARK_LIST) == 0) {
                            shiro_binary* b_arg = __compile_statement(arg, line);

                            int j;
                            bool has_push = false;
                            for (j = 0; j < b_arg->used; j++)
                                if (b_arg->nodes[j]->code == PUSH ||
                                    b_arg->nodes[j]->code == PUSH_BY_NAME)
                                    has_push = true;

                            if (!has_push) {
                                __error(*line, ERR_SYNTAX_ERROR,
                                        "Invalid argument for function '%s':"
                                        " argument doesn't return any value", name);
                                return binary;
                            }

                            free_statement(arg);
                            arg = new_statement(1);
                            concat_and_free_binary(binary, b_arg);
                        } else {
                            push_token(arg, tk);
                            free_token(tk);
                        }
                    }

                    shiro_binary* b_arg = __compile_statement(arg, line);
                    free_statement(arg);

                    int j;
                    bool has_push = false;
                    for (j = 0; j < b_arg->used; j++)
                        if (b_arg->nodes[j]->code == PUSH ||
                            b_arg->nodes[j]->code == PUSH_BY_NAME)
                            has_push = true;

                    if (!has_push) {
                        __error(*line, ERR_SYNTAX_ERROR,
                                "Invalid argument for function '%s':"
                                " argument doesn't return any value", name);
                        return binary;
                    }
                    concat_and_free_binary(binary, b_arg);

                    shiro_node* fcall = new_node(FN_CALL, 1, new_shiro_string(name));
                    push_node(binary, fcall);
                    free_node(fcall);
                }

                token = get_token(statement, 2, line);

                if (token != SHIRO_NIL && strcmp(token->value, MARK_PROP) == 0) {
                    token = get_token(statement, 2, line);

                    if (get_token_type(token) == s_tkName) {
                        shiro_statement* rest = malloc(sizeof(shiro_statement));
                        memcpy(rest, statement, sizeof(shiro_statement));
                        rest->tokens += sizeof(shiro_token*) * 2;
                        rest->allocated -= 2;
                        rest->used -= 2;

                        shiro_binary* b_rest = __compile_statement(rest, line);

                        free(rest);

                        shiro_node* node = new_node(PUSH_BY_NAME, 1, new_shiro_string(name));
                        push_node(binary, node);
                        free_node(node);

                        binary = concat_and_free_binary(binary, b_rest);
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting NAME", token->value);
                        return binary;
                    }
                } else if (token != SHIRO_NIL && strcmp(token->value, MARK_EOS) != 0) {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected %s, expecting <END>", token->value);
                    return binary;
                }
            } else {
                __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                return binary;
            }
            break;
        }
        case s_tkMark:
            break;
        case s_tkConst:
        {
            const shiro_string value = token->value;

            token = get_token(statement, 1, line);
            if (token != SHIRO_NIL) {

            } else {
                shiro_value* s_value = malloc(sizeof(shiro_value));

                if (value[0] == *MARK_STR1 || value[0] == *MARK_STR2) {
                    s_value->type = s_tString;
                    s_value->fields = calloc(2, sizeof(union shiro_field));

                    shiro_string v = malloc(strlen(value) - 1);
                    memset(v, 0, strlen(value) - 1);
                    memcpy(v, value + 1, strlen(value) - 2);

                    s_value->fields[0].str = v;
                    s_value->fields[1].i   = (shiro_fixnum)strlen(s_value->fields[0].str);
                    s_value->n_fields = 2;
                } else {
                    s_value->type = s_tInt;
                    s_value->fields = calloc(1, sizeof(union shiro_field));

                    char* end;
                    s_value->fields[0].i = (shiro_fixnum)strtol(value, &end, 10);
                    s_value->n_fields = 1;
                }

                shiro_node* node = new_node(PUSH, 1, s_value);
                push_node(binary, node);
                free_node(node);
            }
            break;
        }
        default:
            __error(*line, ERR_SYNTAX_ERROR, "Unexpected token '%s'", token->value);
            break;
    }

    return binary;
}
//-----------------------------------------------------------------------------
// Processa uma token no processo de compila��o
//      token   : Ponteiro para a shiro_token sendo processada
//      line    : Ponteiro para o shiro_uint representando o n�mero da linha
//                atual
//      cline   : Ponteiro para o shiro_uint representando o n�mero da linha
//                at� a qual o c�digo foi compilado
//      stack   : N�mero de chaves abertas e n�o fechadas
//      p_stack : N�mero de par�ntesis abertos e n�o fechados
//      stmt    : Ponteiro para o shiro_statement sendo processado
//      binary  : Ponteiro para o shiro_binary que ser� alterado pelo
//                processamento da token
//-----------------------------------------------------------------------------
void __process_token(
    shiro_token*        token,
    shiro_uint*         line,
    shiro_uint*         cline,
    shiro_uint*         stack,
    shiro_uint*         p_stack,
    shiro_statement*    stmt,
    shiro_binary*       binary
) {
    if (token == SHIRO_NIL)
        return;

    if (strcmp(token->value, MARK_EOL) == 0) {
        (*line)++;
    } else if (strcmp(token->value, MARK_OBLOCK) == 0)
        if ((*p_stack) != 0) {
            __error(*line, ERR_SYNTAX_ERROR,
                        "Unexpected '%s', expecting '%s'",
                        MARK_OBLOCK, MARK_CEXPR);
            return;
        } else
            (*stack)++;
    else if (strcmp(token->value, MARK_CBLOCK) == 0) {
        if ((*stack) > 0)
            (*stack)--;
        else {
            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CBLOCK);
            return;
        }
    } else if (strcmp(token->value, MARK_OEXPR) == 0) {
        (*p_stack)++;
    } else if (strcmp(token->value, MARK_CEXPR) == 0) {
        if ((*p_stack) > 0)
            (*p_stack)--;
        else {
            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CEXPR);
            return;
        }
    }
    push_token(stmt, token);

    if (strcmp(token->value, MARK_EOS) == 0 && (*stack) == 0 && (*p_stack) == 0) {
        shiro_binary* other = __compile_statement(stmt, cline);
        binary = concat_and_free_binary(binary, other);

        stmt->used = 0;
        memset(stmt->tokens, 0, stmt->allocated * sizeof(shiro_token*));
    }
}
//-----------------------------------------------------------------------------
// Compila o c�digo passado
//      code    : C�digo
//-----------------------------------------------------------------------------
shiro_binary* shiro_compile(const shiro_string code) {
    shiro_binary* binary = new_binary();

    shiro_statement* tokens = shiro_tokenize(code);

    shiro_statement* statement = new_statement(strlen(code) / 2 + 1);

    shiro_uint   line_compiled = 1,
                 line = 1,
                 stack = 0,
                 p_stack = 0;

    shiro_uint i = 0;
    for (i = 0; i < tokens->used; i++)
        __process_token(
            tokens->tokens[i], &line, &line_compiled, &stack, &p_stack,
            statement, binary
        );
    free_statement(tokens);

    if (stack > 0)
        __error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CBLOCK);
    else if (p_stack > 0)
        __error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CEXPR);

    shiro_binary* other = __compile_statement(statement, &line_compiled);

    binary = concat_and_free_binary(binary, other);

    free_statement(statement);

    return binary;
}
//-----------------------------------------------------------------------------
// Ponto de entrada para teste
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
    static const shiro_string code = "if (1) { print('Hello World'); }; print('asd');";

    double get_time() {
        LARGE_INTEGER t, f;
        QueryPerformanceCounter(&t);
        QueryPerformanceFrequency(&f);
        return (double)t.QuadPart/(double)f.QuadPart;
    }

    double t0 = get_time();
    shiro_binary* bin = shiro_compile(code);
    double d = (get_time() - t0) * 1000;

    printf("Code compiled succesfully, output has %d nodes\n", bin->used);
    int i;
    for (i = 0; i < bin->used; i++) {
        shiro_node* node = bin->nodes[i];
        printf("0x%x ", node->code);
        int j;
        for (j = 0; j < node->n_args; j++)
            printf("%s ", node->args[j]->fields[0].str);
    }
    printf("\n");
    printf("Compilation has taken about %f milliseconds\n", d);

    free_binary(bin);

    return 0;
}
