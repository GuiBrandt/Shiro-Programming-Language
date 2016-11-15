//=============================================================================
// src\compile.c
//-----------------------------------------------------------------------------
// Define as funções usadas para compilar código Shiro
//=============================================================================
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "vm.h"
#include "lexer.h"
#include "parser.h"
#include "errors.h"

shiro_statement*    __expression        (const shiro_statement*, shiro_uint);
shiro_statement*    __block             (const shiro_statement*, shiro_uint);
shiro_binary*       __compile_statements(const shiro_statement*, shiro_uint*);
shiro_binary*       __compile_statement (const shiro_statement*, shiro_uint*);

//-----------------------------------------------------------------------------
// Lança uma mensagem de erro e aborta a execução do programa
//      line        : Linha do erro
//      errcode     : Nome do tipo de erro
//      message     : Mensagem de erro
//      ...         : Parâmetros usados para formatação
//-----------------------------------------------------------------------------
SHIRO_API void shiro_error(
    const shiro_uint line,
    const shiro_string errcode,
    const shiro_string message,
    ...
) {
    char* err = malloc(1024);
    sprintf(err, "%s on line %d: %s", errcode, line, message);

    va_list args;
    va_start(args, message);

    char* error = malloc(1024);
    vsprintf(error, err, args);
    va_end(args);

    last_error = error;
}
//-----------------------------------------------------------------------------
// Obtém a última mensagem de erro lançada pelo shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_string shiro_get_last_error() {
    shiro_string err = last_error;
    last_error = NULL;
    return err;
}
//-----------------------------------------------------------------------------
// Obtém uma expressão a partir de um token '('
//      stmt    : Ponteiro para o shiro_statement onde está o token '('
//      offset  : Índice do token no shiro_statement
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

        if (strcmp(token->value, MARK_EOS) == 0) {
            shiro_error(line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_EOS);
            return NULL;
        }

        if (p_stack > 0)
            push_token(expression, token);

        if (++i >= stmt->used)
            break;

        token = stmt->tokens[i];
    }

    return expression;
}
//-----------------------------------------------------------------------------
// Obtém um bloco de código a partir de um token '{'
//      stmt    : Ponteiro para o shiro_statement onde está o token '{'
//      offset  : Índice do token no shiro_statement
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

        if (++i >= stmt->used)
            break;

        token = stmt->tokens[i];
    }

    return expression;
}
//-----------------------------------------------------------------------------
// Compila um operador
//      binary      : Binário com o operando da esquerda
//      op          : Operador
//      right_hand  : Binário com o operando da direita
//      line        : Linha
//-----------------------------------------------------------------------------
shiro_binary* __compile_operator(
    shiro_binary* binary,
    shiro_string op,
    shiro_binary* right_hand,
    shiro_uint* line
) {
    if (!binary_returns_value(right_hand)) {
        shiro_error(*line, ERR_SYNTAX_ERROR, "Invalid right hand operand: operand doesn't have any value");
        return NULL;
    }

    shiro_bytecode code = DIE;

    if (strcmp(op, OP_ADD) == 0) {
        code = ADD;
    } else if (strcmp(op, OP_SUB) == 0) {
        code = SUB;
    } else if (strcmp(op, OP_MUL) == 0) {
        code = MUL;
    } else if (strcmp(op, OP_DIV) == 0) {
        code = DIV;
    } else if (strcmp(op, OP_MOD) == 0) {
        code = MOD;
    } else if (strcmp(op, OP_AND) == 0) {
        code = B_AND;
    } else if (strcmp(op, OP_OR) == 0) {
        code = B_OR;
    } else if (strcmp(op, OP_XOR) == 0) {
        code = B_XOR;
    } else if (strcmp(op, CMP_EQU) == 0 || strcmp(op, CMP_DIF) == 0) {
        code = COMPARE_EQ;
    } else if (strcmp(op, CMP_GRT) == 0) {
        code = COMPARE_GT;
    } else if (strcmp(op, CMP_LT) == 0) {
        code = COMPARE_LT;
    } else if (strcmp(op, CMP_GRTEQU) == 0) {
        code = COMPARE_GT_EQ;
    } else if (strcmp(op, CMP_LTEQU) == 0) {
        code = COMPARE_LT_EQ;
    } else {
        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected operator '%s'", op);
        return NULL;
    }

    concat_and_free_binary(binary, right_hand);

    shiro_node* operate = new_node(code, 0);
    push_node(binary, operate);
    free_node(operate);

    if (strcmp(op, CMP_DIF) == 0) {
        shiro_node* invert = new_node(NOT, 0);
        push_node(binary, invert);
        free_node(invert);
    }

    return binary;
}
//-----------------------------------------------------------------------------
// Compila uma expressão
//      statement   : Expressão
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
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }
            }

            //
            //  if (<expr>) { <code> } [else { <code> }]
            //
            else if (strcmp(token->value, KW_COND) == 0) {
                token = get_token(statement, 1, line);

                if (token != NULL && strcmp(token->value, MARK_OEXPR) == 0) {
                    shiro_protect(
                        shiro_statement* expression = __expression(statement, 1)
                    );

                    shiro_protect(
                        shiro_binary* b_expr = __compile_statements(expression, line);
                    );
                    free_statement(expression);

                    if (!binary_returns_value(b_expr)) {
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Invalid conditional statement");
                        return NULL;
                    }

                    token = get_token(statement, 2, line);
                    if (token != NULL && strcmp(token->value, MARK_OBLOCK) == 0) {
                        shiro_statement* block = __block(statement, 2);
                        shiro_protect(
                            shiro_binary* b_block = __compile_statement(block, line);
                        );
                        free_statement(block);

                        token = get_token(statement, 3, line);
                        if (token != NULL && strcmp(token->value, KW_ELSE) == 0) {

                            token = get_token(statement, 4, line);

                            if (token != NULL && strcmp(token->value, MARK_OBLOCK) == 0)
                                if (get_token(statement, 5, line) == NULL) {
                                    shiro_statement* block2 = __block(statement, 4);
                                    shiro_protect(
                                        shiro_binary* b_block2 = __compile_statements(block2, line);
                                    );
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
                                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                                    return NULL;
                                }
                            else {
                                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                                return NULL;
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
                            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                            return NULL;
                        }
                    } else {
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                        return NULL;
                    }
                } else {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OEXPR);
                    return NULL;
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
                        shiro_protect(
                            shiro_binary* b_val = __compile_statement(val_stmt, line);
                        );
                        free_statement(val_stmt);

                        if (!binary_returns_value(b_val)) {
                            shiro_error(*line, ERR_SYNTAX_ERROR, "Invalid variable value: statement doesn't have any value");
                            return NULL;
                        }

                        binary = concat_and_free_binary(binary, b_val);

                        shiro_node* set = new_node(SET_VAR, 1, shiro_new_uint(ID(name)));
                        push_node(binary, set);
                        free_node(set);
                    } else {
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END> or '%s'", token->value, OP_SET);
                        return NULL;
                    }
                } else {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                    return NULL;
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
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }
                } else {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                    return NULL;
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
                        shiro_protect(
                            shiro_statement* args = __expression(statement, 2);
                        );

                        shiro_token* tk;
                        shiro_uint i, n = 0;
                        for (i = 0; i < args->used; i += 2, n++) {

                            tk = get_token(args, i, line);
                            if (tk == NULL) {
                                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting <NAME>");
                                return NULL;
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

                                    continue;
                                }

                                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", tk->value, MARK_LIST);
                                return NULL;
                            } else {
                                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", tk->value);
                                return NULL;
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
                            shiro_protect(
                                shiro_binary* b_block = __compile_statements(block, line);
                            );
                            concat_and_free_binary(bin, b_block);

                            if (!binary_returns_value(bin)) {
                                shiro_node* ret = new_node(PUSH_BY_NAME, 1, ID("nil"));
                                bin = push_node(bin, ret);
                                free_node(ret);
                            }

                            shiro_function* fn = shiro_new_fn(b_args->used / 2, bin);
                            shiro_free_binary(b_args);

                            shiro_node* set = new_node(SET_FN, 2, shiro_new_uint(ID(name)), shiro_new_function(fn));
                            push_node(binary, set);
                            free_node(set);
                        } else {
                            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                            return NULL;
                        }

                        free_statement(block);
                    } else {
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                        return NULL;
                    }
                } else {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                    return NULL;
                }
            }

            //
            //  include "<filename>"
            //
            else if (strcmp(token->value, KW_INCLUDE) == 0) {

                token = get_token(statement, 1, line);

                if (token == NULL) {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting STRING");
                    return NULL;
                }

                if (get_token_type(token) != s_tkConst || (*token->value != *MARK_STR1 && *token->value != *MARK_STR2)) {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting STRING", token->value);
                    return NULL;
                }

                shiro_uint len = strlen(token->value) - 2;
                shiro_string filename = calloc(len + 1, sizeof(shiro_character));
                memcpy(filename, token->value + 1, len);

                token = get_token(statement, 2, line);

                if (token == NULL || strcmp(token->value, MARK_EOS) == 0) {

                    if (strrchr(filename, '.') <= strrchr(filename, '/') ||
                        strrchr(filename, '.') <= strrchr(filename, '\\'))
                        strcat(filename, ".shiro");

                    FILE* file = fopen(filename, "r");

                    if (file == NULL) {
                        shiro_error(0, "IOError", "No such file or directory '%s'", filename);
                        return NULL;
                    }

                    fseek(file, 0, SEEK_END);
                    shiro_uint size = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    shiro_string fcontents = calloc(size, sizeof(shiro_character));
                    fread(fcontents, sizeof(shiro_character), size, file);

                    shiro_protect(
                        shiro_binary* bin = shiro_compile(fcontents);
                    );

                    binary = concat_and_free_binary(binary, bin);

                    return binary;
                } else {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }
            }

            //
            //  import "<filename>"
            //
            else if (strcmp(token->value, KW_IMPORT) == 0) {
                token = get_token(statement, 1, line);

                if (token == NULL) {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting STRING");
                    return NULL;
                }

                if (get_token_type(token) == s_tkConst && (*token->value == *MARK_STR1 || *token->value == *MARK_STR2)) {
                    shiro_uint len = strlen(token->value) - 2;
                    shiro_string filename = calloc(len + 1, sizeof(shiro_character));
                    memcpy(filename, token->value + 1, len);

                    token = get_token(statement, 2, line);

                    if (token == NULL || strcmp(token->value, MARK_EOS) == 0) {
                        shiro_node* push = new_node(PUSH, 1, shiro_new_string(filename));
                        push_node(binary, push);
                        free_node(push);

                        shiro_node* call = new_node(FN_CALL, 2, shiro_new_uint(ID("import")), shiro_new_uint(1));
                        push_node(binary, call);
                        free_node(call);

                        return binary;
                    } else {
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }

                    free(filename);
                }

                token = get_token(statement, 0, line);
            }

            if (token == NULL || (
                strcmp(token->value, KW_NIL) != 0 &&
                strcmp(token->value, KW_SELF) != 0 &&
                strcmp(token->value, KW_IMPORT) != 0))
                break;
        }
        case s_tkName:
        {
            shiro_string name = token->value;

            token = get_token(statement, 1, line);

            if (token == NULL) {
                shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
                push_node(binary, node);
                free_node(node);
            } else if (get_token_type(token) == s_tkBinaryOperator) {
                shiro_statement* rest = offset_statement(statement, 2);

                shiro_protect(
                    shiro_binary* b_val = __compile_statement(rest, line);
                );
                free_statement(rest);

                shiro_string op = token->value;

                shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
                push_node(binary, node);
                free_node(node);

                shiro_protect(
                    binary = __compile_operator(binary, op, b_val, line);
                );

                return binary;

            } else if (strcmp(token->value, MARK_PROP) == 0) {
                /*token = get_token(statement, 2, line);

                if (get_token_type(token) == s_tkName) {
                    shiro_statement* rest = offset_statement(statement, 2);

                    shiro_binary* b_rest = __compile_statement(rest, line);

                    free_statement(rest);

                    shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
                    push_node(binary, node);
                    free_node(node);

                    binary = concat_and_free_binary(binary, b_rest);
                } else {
                    __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting NAME", token->value);
                    return NULL;
                }*/
                shiro_error(*line, "NotSupportedYet", "Properties are not supported yet");
                return NULL;
            } else if (strcmp(token->value, MARK_OEXPR) == 0) {
                shiro_protect(
                    shiro_statement* expression = __expression(statement, 1);
                );
                {
                    shiro_uint n_args = 0;
                    shiro_statement* arg = new_statement(1);
                    shiro_token* tk = expression->tokens[0];

                    shiro_binary* bin = new_binary();

                    int i;
                    for (i = 0; i < expression->used; i++) {
                        tk = expression->tokens[i];
                        if (strcmp(tk->value, MARK_LIST) == 0) {
                            shiro_protect(
                                shiro_binary* b_arg = __compile_statement(arg, line);
                            );

                            if (!binary_returns_value(b_arg)) {
                                shiro_error(*line, ERR_SYNTAX_ERROR,
                                        "Invalid argument for function '%s':"
                                        " argument doesn't have any value", name);
                                return NULL;
                            }

                            free_statement(arg);
                            arg = new_statement(1);
                            n_args++;
                            bin = concat_and_free_binary(b_arg, bin);
                        } else {
                            if (strcmp(tk->value, MARK_EOL) == 0)
                                (*line)++;
                            push_token(arg, tk);
                        }
                    }
                    shiro_protect(
                        shiro_binary* b_arg = __compile_statement(arg, line);
                    );
                    free_statement(arg);

                    if (b_arg->used > 0) {
                        if (!binary_returns_value(b_arg)) {
                            shiro_error(*line, ERR_SYNTAX_ERROR,
                                    "Invalid argument for function '%s':"
                                    " argument doesn't return any value", name);
                            return NULL;
                        }
                        concat_and_free_binary(binary, b_arg);
                        n_args++;
                    } else if (n_args > 0) {
                        shiro_error(*line, ERR_SYNTAX_ERROR,
                                "Unexpected <END>, expecting <VALUE>");
                        return NULL;
                    } else
                        shiro_free_binary(b_arg);

                    binary = concat_and_free_binary(binary, bin);

                    shiro_node* fcall = new_node(FN_CALL, 2, shiro_new_uint(ID(name)), shiro_new_uint(n_args));
                    push_node(binary, fcall);
                    free_node(fcall);
                }
                free_statement(expression);

                token = get_token(statement, 2, line);

                if (token != NULL && strcmp(token->value, MARK_PROP) == 0) {
                    /*token = get_token(statement, 2, line);

                    if (get_token_type(token) == s_tkName) {
                        shiro_statement* rest = offset_statement(statement, 2);
                        shiro_protect(
                            shiro_binary* b_rest = __compile_statement(rest, line);
                        );

                        free(rest);

                        shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_string(name));
                        push_node(binary, node);
                        free_node(node);

                        binary = concat_and_free_binary(binary, b_rest);
                    } else {
                        __error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting NAME", token->value);
                        return NULL;
                    }*/
                    shiro_error(*line, "NotSupportedYet", "Properties are not supported yet");
                } else if (get_token_type(token) == s_tkBinaryOperator) {
                    shiro_statement* rest = offset_statement(statement, 3);

                    shiro_protect(
                        shiro_binary* b_val = __compile_statement(rest, line);
                    );
                    free_statement(rest);
                    shiro_string op = token->value;

                    /*shiro_node* fcall = new_node(FN_CALL, 2, shiro_new_uint(ID(name)), shiro_new_uint(n_args));
                    push_node(binary, fcall);
                    free_node(fcall);*/

                    shiro_protect(
                        binary = __compile_operator(binary, op, b_val, line);
                    );
                    return binary;
                } else if (token != NULL && strcmp(token->value, MARK_EOS) != 0) {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }
            } else {
                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                return NULL;
            }
            break;
        }
        case s_tkMark:
        {
            if (strcmp(token->value, MARK_OEXPR) == 0) {
                shiro_protect(
                    shiro_statement* expr = __expression(statement, 0);
                );
                shiro_protect(
                    shiro_binary* bin = __compile_statement(expr, line);
                );
                binary = concat_and_free_binary(binary, bin);
                free_statement(expr);

                token = get_token(statement, 1, line);
                if (token == NULL) {
                    return binary;
                } else if (get_token_type(token) == s_tkBinaryOperator) {
                    shiro_statement* rest = offset_statement(statement, 2);

                    shiro_protect(
                        shiro_binary* b_val = __compile_statement(rest, line);
                    );
                    free_statement(rest);
                    shiro_string op = token->value;

                    shiro_protect(
                        binary = __compile_operator(binary, op, b_val, line);
                    );
                    return binary;
                } else {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }
            } else if (strcmp(token->value, MARK_OBLOCK) == 0) {
                shiro_statement* block = __block(statement, 0);
                shiro_protect(
                    shiro_binary* bin = __compile_statement(block, line);
                );
                binary = concat_and_free_binary(binary, bin);
                free_statement(block);

                token = get_token(statement, 1, line);
                if (token == NULL)
                    return binary;
                else {
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }
            } else if (strcmp(token->value, MARK_EOS) == 0)
                return binary;
            else {
                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", token->value);
                return NULL;
            }
            break;
        }
        case s_tkConst:
        {
            const shiro_string value = token->value;

            void __push_val() {
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

            token = get_token(statement, 1, line);
            if (token == NULL || strcmp(token->value, MARK_EOS) == 0) {
                __push_val();
            } else if (get_token_type(token) == s_tkBinaryOperator) {
                shiro_statement* rest = offset_statement(statement, 2);

                shiro_protect(
                    shiro_binary* b_val = __compile_statement(rest, line);
                );
                free_statement(rest);
                shiro_string op = token->value;
                __push_val();

                shiro_protect(
                    binary = __compile_operator(binary, op, b_val, line);
                );

                return binary;
            } else {
                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                return NULL;
            }
            break;
        }
        default:
            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected token '%s'", token->value);
            return NULL;
    }

    return binary;
}
//-----------------------------------------------------------------------------
// Processa uma token no processo de compilação
//      token   : Ponteiro para a shiro_token sendo processada
//      line    : Ponteiro para o shiro_uint representando o número da linha
//                atual
//      cline   : Ponteiro para o shiro_uint representando o número da linha
//                até a qual o código foi compilado
//      stack   : Número de chaves abertas e não fechadas
//      p_stack : Número de parêntesis abertos e não fechados
//      stmt    : Ponteiro para o shiro_statement sendo processado
//      binary  : Ponteiro para o shiro_binary que será alterado pelo
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
            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CBLOCK);
            return NULL;
        }
    } else if (strcmp(token->value, MARK_OEXPR) == 0) {
        (*p_stack)++;
    } else if (strcmp(token->value, MARK_CEXPR) == 0) {
        if ((*p_stack) > 0)
            (*p_stack)--;
        else {
            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s'", MARK_CEXPR);
            return NULL;
        }
    }

    if (strcmp(token->value, MARK_EOS) == 0 && (*stack) == 0 && (*p_stack) == 0) {
        shiro_protect(
            shiro_binary* other = __compile_statement(stmt, cline);
        );
        binary = concat_and_free_binary(binary, other);

        shiro_uint size = stmt->allocated;
        free_statement(stmt);
        stmt = new_statement(size);
    } else
        push_token(stmt, token);

    return stmt;
}
//-----------------------------------------------------------------------------
// Compila mais de uma expressão
//      statements  : Expressões
//      cline       : Ponteiro para o número da linha
//-----------------------------------------------------------------------------
shiro_binary* __compile_statements(
    const shiro_statement* statements,
    shiro_uint* cline
) {
    shiro_statement* statement = new_statement(statements->used);
    shiro_binary* binary = new_binary();

    shiro_uint line = 0, p_stack = 0, stack = 0, i;
    for (i = 0; i < statements->used; i++)
        shiro_protect(
            statement = __process_token(
                statements->tokens[i],
                &line,
                cline,
                &stack,
                &p_stack,
                statement,
                binary
            );
        );

    if (stack > 0) {
        shiro_error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CBLOCK);
        return NULL;
    }
    else if (p_stack > 0) {
        shiro_error(line, ERR_SYNTAX_ERROR,
                    "Unexpected <EOF>, expecting '%s'", MARK_CEXPR);
        return NULL;
    }

    shiro_protect(
        shiro_binary* other = __compile_statement(statement, cline);
    );

    if (shiro_get_last_error() != NULL)
        return NULL;

    binary = concat_and_free_binary(binary, other);
    free_statement(statement);

    return binary;
}
//-----------------------------------------------------------------------------
// Compila o código passado
//      code    : Código
//-----------------------------------------------------------------------------
SHIRO_API shiro_binary* shiro_compile(const shiro_string code) {
    shiro_statement* tokens = shiro_tokenize(code);

    shiro_uint line_compiled = 1;
    shiro_protect(
        shiro_binary* binary = __compile_statements(tokens, &line_compiled);
    );
    free_statement(tokens);

    return binary;
}
