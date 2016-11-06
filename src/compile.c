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

shiro_statement*    __expression        (const shiro_statement*, shiro_uint);
shiro_statement*    __block             (const shiro_statement*, shiro_uint);
shiro_binary*       __compile_statements(const shiro_statement*, shiro_uint*);
shiro_binary*       __compile_statement (const shiro_statement*, shiro_uint*);

//-----------------------------------------------------------------------------
// Lan�a uma mensagem de erro e aborta a execu��o do programa
//      line        : Linha do erro
//      errcode     : Nome do tipo de erro
//      message     : Mensagem de erro
//      ...         : Par�metros usados para formata��o
//-----------------------------------------------------------------------------
void __error(
    const shiro_uint line,
    const shiro_string errcode,
    const shiro_string message,
    ...
) {
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

    shiro_uint p_stack = 1, line = 1;
    shiro_token* token = get_token(stmt, offset, &line);

    shiro_uint i = 0;
    while (stmt->tokens[i] != token && i < stmt->used)
        i++;

    token = stmt->tokens[++i];

    while (p_stack > 0) {
        if (strcmp(token->value, MARK_OEXPR) == 0)
            p_stack++;
        else if (strcmp(token->value, MARK_CEXPR) == 0)
            p_stack--;
        else if (strcmp(token->value, MARK_EOL) == 0)
            line++;

        if (strcmp(token->value, MARK_EOS) == 0)
            __error(line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_EOS);

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

    shiro_uint stack = 1, line = 1;
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

    if (statement == NULL || statement->used == 0)
        return binary;

    const shiro_token* token;
    token = get_token(statement, 0, line);

    if (token == NULL)
        return binary;

    switch (get_token_type(token)) {
        case s_tkKeyword:
        {
            //
            //  exit
            //
            if (strcmp(token->value, KW_DIE) == 0) {
                token = get_token(statement, 1, line);
                if (token == NULL) {
                    shiro_node* node = new_node(DIE, 0);
                    push_node(binary, node);
                    free_node(node);
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return binary;
                }


            }

            //
            //  if (<expr>) { <code> } [else { <code> }]
            //
            else if (strcmp(token->value, KW_COND) == 0) {
                token = get_token(statement, 1, line);

                if (token != NULL && strcmp(token->value, MARK_OEXPR) == 0) {
                    shiro_statement* expression = __expression(statement, 1);
                    shiro_binary* b_expr = __compile_statements(expression, line);
                    free_statement(expression);

                    if (!binary_returns_value(b_expr))
                        __error(*line, ERR_SYNTAX_ERROR, "Invalid conditional statement");

                    token = get_token(statement, 2, line);
                    if (token != NULL && strcmp(token->value, MARK_OBLOCK) == 0) {
                        shiro_statement* block = __block(statement, 2);
                        shiro_binary* b_block = __compile_statement(block, line);
                        free_statement(block);

                        token = get_token(statement, 3, line);
                        if (token != NULL && strcmp(token->value, KW_ELSE) == 0) {

                            token = get_token(statement, 4, line);

                            if (token != NULL && strcmp(token->value, MARK_OBLOCK) == 0)
                                if (get_token(statement, 5, line) == NULL) {
                                    shiro_statement* block2 = __block(statement, 4);
                                    shiro_binary* b_block2 = __compile_statements(block2, line);
                                    free_statement(block2);

                                    shiro_binary* bin = new_binary();

                                    shiro_node* cond = new_node(COND, 0);
                                    push_node(bin, cond);
                                    free_node(cond);

                                    shiro_node* jmp0 = new_node(JUMP, 1, shiro_new_fixnum(1));
                                    push_node(bin, jmp0);
                                    free_node(jmp0);

                                    shiro_node* jmp1 = new_node(JUMP, 1, shiro_new_fixnum(b_block2->used + 1));
                                    push_node(bin, jmp1);
                                    free_node(jmp1);

                                    bin = concat_and_free_binary(bin, b_block2);

                                    shiro_node* jmp2 = new_node(JUMP, 1, shiro_new_fixnum(b_block->used));
                                    push_node(bin, jmp2);
                                    free_node(jmp2);

                                    bin = concat_and_free_binary(b_expr, bin);
                                    bin = concat_and_free_binary(bin, b_block);
                                    binary = concat_and_free_binary(binary, bin);
                                } else {
                                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                                    return binary;
                                }
                            else {
                                __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                                return binary;
                            }
                        } else if (token == NULL) {

                            shiro_binary* bin = new_binary();

                            shiro_node* cond = new_node(COND, 0);
                            push_node(bin, cond);
                            free_node(cond);

                            shiro_node* jmp = new_node(JUMP, 1, shiro_new_fixnum(b_block->used));
                            push_node(bin, jmp);
                            free_node(jmp);

                            bin = concat_and_free_binary(b_expr, bin);
                            bin = concat_and_free_binary(bin, b_block);
                            binary = concat_and_free_binary(binary, bin);
                        } else {
                            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
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

            //
            //  var <name>[ = <value>]
            //
            else if (strcmp(token->value, KW_VAR) == 0) {
                token = get_token(statement, 1, line);

                if (get_token_type(token) == s_tkName) {
                    shiro_string name = token->value;

                    token = get_token(statement, 2, line);
                    if (token == NULL) {

                        shiro_node* alloc = new_node(ALLOC, 1, shiro_new_uint(ID(name)));
                        push_node(binary, alloc);
                        free_node(alloc);

                    } else if (strcmp(token->value, OP_SET) == 0) {
                        shiro_statement* val_stmt = offset_statement(statement, 3);
                        shiro_binary* b_val = __compile_statement(val_stmt, line);
                        free_statement(val_stmt);

                        if (!binary_returns_value(b_val)) {
                            __error(*line, ERR_SYNTAX_ERROR, "Invalid variable value: statement doesn't have any value");
                            return binary;
                        }

                        binary = concat_and_free_binary(binary, b_val);

                        shiro_node* set = new_node(SET_VAR, 1, shiro_new_uint(ID(name)));
                        push_node(binary, set);
                        free_node(set);
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END> or '%s'", token->value, OP_SET);
                        return binary;
                    }
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                    return binary;
                }
            }

            //
            //  delete <name>
            //
            else if (strcmp(token->value, KW_DELETE) == 0) {
                token = get_token(statement, 1, line);

                if (get_token_type(token) == s_tkName) {
                    shiro_string name = token->value;

                    token = get_token(statement, 2, line);
                    if (token == NULL) {
                        shiro_node* del = new_node(FREE, 1, shiro_new_uint(ID(name)));
                        push_node(binary, del);
                        free_node(del);

                        return binary;
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return binary;
                    }
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                    return binary;
                }
            }

            //
            //  fn <name>[([<name>[, <name>[, ...]])] { <code> }
            //
            else if (strcmp(token->value, KW_FUNC) == 0) {
                token = get_token(statement, 1, line);

                if (get_token_type(token) == s_tkName) {
                    shiro_string name = token->value;

                    token = get_token(statement, 2, line);

                    shiro_binary* bin = new_binary();

                    int tk_i = 2;
                    shiro_binary* b_args = new_binary();
                    if (strcmp(token->value, MARK_OEXPR) == 0) {
                        shiro_statement* args = __expression(statement, 2);

                        shiro_token* tk;
                        shiro_uint i, n = 0;
                        for (i = 0; i < args->used; i += 2, n++) {

                            tk = get_token(args, i, line);
                            if (tk == NULL) {
                                __error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting <NAME>");
                                return binary;
                            } else if (get_token_type(tk) == s_tkName) {
                                shiro_string name = tk->value;

                                tk = get_token(args, i + 1, line);
                                if (tk == NULL || strcmp(tk->value, MARK_LIST) == 0) {
                                    shiro_node* arg_push = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ARG(n)));
                                    push_node(b_args, arg_push);
                                    free_node(arg_push);

                                    shiro_node* set = new_node(SET_VAR, 1, shiro_new_uint(ID(name)));
                                    push_node(b_args, set);
                                    free_node(set);

                                    shiro_node* drop = new_node(DROP, 0);
                                    push_node(b_args, drop);
                                    free_node(drop);

                                    continue;
                                }

                                __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", tk->value, MARK_LIST);
                                break;
                            } else {
                                __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", tk->value);
                                return binary;
                            }
                        }
                        free_statement(args);

                        token = get_token(statement, 3, line);
                        tk_i++;
                    }

                    if (strcmp(token->value, MARK_OBLOCK) == 0) {
                        shiro_statement* block = __block(statement, tk_i);
                        token = get_token(statement, tk_i + 1, line);

                        if (token == NULL) {
                            concat_binary(bin, b_args);

                            shiro_binary* b_block = __compile_statements(block, line);
                            concat_and_free_binary(bin, b_block);

                            shiro_function* fn = malloc(sizeof(shiro_function));
                            fn->type = s_fnShiroBinary;
                            fn->n_args = b_args->used / 3;
                            fn->s_binary = bin;

                            free_binary(b_args);

                            shiro_node* set = new_node(SET_FN, 2, shiro_new_uint(ID(name)), shiro_new_function(fn));
                            push_node(binary, set);
                            free_node(set);
                        } else {
                            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                            return binary;
                        }

                        free_statement(block);
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                        return binary;
                    }
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                    return binary;
                }
            }


            if (token == NULL || (strcmp(token->value, KW_NIL) != 0 && strcmp(token->value, KW_SELF) != 0))
                break;
        }
        case s_tkName:
        {
            shiro_string name = calloc(token->used + 1, sizeof(shiro_character));
            memcpy(name, token->value, token->used * sizeof(shiro_character));

            token = get_token(statement, 1, line);

            if (token == NULL) {
                shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
                push_node(binary, node);
                free_node(node);
            } else if (strcmp(token->value, MARK_PROP) == 0) {
                token = get_token(statement, 2, line);

                if (get_token_type(token) == s_tkName) {
                    shiro_statement* rest = offset_statement(statement, 2);

                    shiro_binary* b_rest = __compile_statement(rest, line);

                    free(rest);

                    shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
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
                    shiro_uint n_args = 0;
                    shiro_statement* arg = new_statement(1);
                    shiro_token* tk = expression->tokens[0];

                    int i;
                    for (i = 0; i < expression->used; i++) {
                        tk = expression->tokens[i];
                        if (strcmp(tk->value, MARK_LIST) == 0) {
                            shiro_binary* b_arg = __compile_statement(arg, line);

                            if (!binary_returns_value(b_arg)) {
                                __error(*line, ERR_SYNTAX_ERROR,
                                        "Invalid argument for function '%s':"
                                        " argument doesn't have any value", name);
                                return binary;
                            }

                            free_statement(arg);
                            arg = new_statement(1);
                            n_args++;
                            concat_and_free_binary(binary, b_arg);
                        } else {
                            if (strcmp(tk->value, MARK_EOL) == 0)
                                (*line)++;
                            push_token(arg, tk);
                        }
                    }

                    shiro_binary* b_arg = __compile_statement(arg, line);
                    free_statement(arg);

                    if (b_arg->used > 0) {
                        if (!binary_returns_value(b_arg)) {
                            __error(*line, ERR_SYNTAX_ERROR,
                                    "Invalid argument for function '%s':"
                                    " argument doesn't return any value", name);
                            return binary;
                        }
                        concat_and_free_binary(binary, b_arg);
                        n_args++;
                    } else if (n_args > 0) {
                        __error(*line, ERR_SYNTAX_ERROR,
                                "Unexpected <END>, expecting <VALUE>");
                        return binary;
                    } else
                        free_binary(b_arg);

                    shiro_node* fcall = new_node(FN_CALL, 2, shiro_new_uint(ID(name)), shiro_new_uint(n_args));
                    push_node(binary, fcall);
                    free_node(fcall);
                }
                free_statement(expression);

                token = get_token(statement, 2, line);

                if (token != NULL && strcmp(token->value, MARK_PROP) == 0) {
                    token = get_token(statement, 2, line);

                    if (get_token_type(token) == s_tkName) {
                        shiro_statement* rest = offset_statement(statement, 2);
                        shiro_binary* b_rest = __compile_statement(rest, line);

                        free(rest);

                        shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_string(name));
                        push_node(binary, node);
                        free_node(node);

                        binary = concat_and_free_binary(binary, b_rest);
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting NAME", token->value);
                        return binary;
                    }
                } else if (token != NULL && strcmp(token->value, MARK_EOS) != 0) {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return binary;
                }
            } else {
                __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                return binary;
            }

            free(name);
            break;
        }
        case s_tkMark:
        {
            if (strcmp(token->value, MARK_OEXPR) == 0) {
                shiro_statement* expr = __expression(statement, 0);
                binary = concat_and_free_binary(binary, __compile_statement(expr, line));
                free_statement(expr);
                return binary;
            } else if (strcmp(token->value, MARK_OBLOCK) == 0) {
                shiro_statement* block = __block(statement, 0);
                binary = concat_and_free_binary(binary, __compile_statement(block, line));
                free_statement(block);
                return binary;
            } else if (strcmp(token->value, MARK_EOS) == 0)
                return binary;
            else
                __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", token->value);
            break;
        }
        case s_tkConst:
        {
            const shiro_string value = token->value;

            token = get_token(statement, 1, line);
            if (token != NULL) {
                __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                return binary;
            } else {
                shiro_value* s_value;

                if (value[0] == *MARK_STR1 || value[0] == *MARK_STR2) {
                    shiro_uint len = strlen(value);
                    shiro_string v = calloc(len - 1, sizeof(shiro_character));
                    memcpy(v, value + 1, len - 2);
                    s_value = shiro_new_string(v);
                    free(v);
                } else {
                    char* end;
                    shiro_fixnum i = (shiro_fixnum)strtol(value, &end, 10);

                    s_value = shiro_new_fixnum(i);
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
shiro_statement* __process_token(
    shiro_token*        token,
    shiro_uint*         line,
    shiro_uint*         cline,
    shiro_uint*         stack,
    shiro_uint*         p_stack,
    shiro_statement*    stmt,
    shiro_binary*       binary
) {
    if (token == NULL)
        return stmt;

    if (strcmp(token->value, MARK_EOL) == 0) {
        (*line)++;
    } else if (strcmp(token->value, MARK_OBLOCK) == 0)
        (*stack)++;
    else if (strcmp(token->value, MARK_CBLOCK) == 0) {
        if ((*stack) > 0)
            (*stack)--;
        else {
            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CBLOCK);
            return stmt;
        }
    } else if (strcmp(token->value, MARK_OEXPR) == 0) {
        (*p_stack)++;
    } else if (strcmp(token->value, MARK_CEXPR) == 0) {
        if ((*p_stack) > 0)
            (*p_stack)--;
        else {
            __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CEXPR);
            return stmt;
        }
    }

    if (strcmp(token->value, MARK_EOS) == 0 && (*stack) == 0 && (*p_stack) == 0) {
        shiro_binary* other = __compile_statement(stmt, cline);
        binary = concat_and_free_binary(binary, other);

        shiro_uint size = stmt->allocated;
        free_statement(stmt);
        stmt = new_statement(size);
    } else
        push_token(stmt, token);

    return stmt;
}
//-----------------------------------------------------------------------------
// Compila mais de uma express�o
//      statements  : Express�es
//      cline       : Ponteiro para o n�mero da linha
//-----------------------------------------------------------------------------
shiro_binary* __compile_statements(
    const shiro_statement* statements,
    shiro_uint* cline
) {
    shiro_statement* statement = new_statement(statements->used);
    shiro_binary* binary = new_binary();

    shiro_uint line = 0, p_stack = 0, stack = 0, i;
    for (i = 0; i < statements->used; i++)
        statement = __process_token(
            statements->tokens[i],
            &line,
            cline,
            &stack,
            &p_stack,
            statement,
            binary
        );

    if (stack > 0)
        __error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CBLOCK);
    else if (p_stack > 0)
        __error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CEXPR);

    shiro_binary* other = __compile_statement(statement, cline);
    binary = concat_and_free_binary(binary, other);
    free_statement(statement);

    return binary;
}
//-----------------------------------------------------------------------------
// Compila o c�digo passado
//      code    : C�digo
//-----------------------------------------------------------------------------
SHIRO_API shiro_binary* shiro_compile(const shiro_string code) {
    shiro_statement* tokens = shiro_tokenize(code);

    shiro_uint line_compiled = 1;
    shiro_binary* binary = __compile_statements(tokens, &line_compiled);
    free_statement(tokens);

    return binary;
}
