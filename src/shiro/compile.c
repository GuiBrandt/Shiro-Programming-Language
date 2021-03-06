//=============================================================================
// src\compile.c
//-----------------------------------------------------------------------------
// Define as fun��es usadas para compilar c�digo Shiro
//=============================================================================
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "vm.h"
#include "lexer.h"
#include "parser.h"
#include "errors.h"

shiro_statement*    __expression        (const shiro_statement*, shiro_uint, shiro_uint);
shiro_statement*    __block             (const shiro_statement*, shiro_uint, shiro_uint);
shiro_binary*       __compile_statements(const shiro_statement*, shiro_uint*);
shiro_binary*       __compile_statement (const shiro_statement*, shiro_uint*);

//-----------------------------------------------------------------------------
// Lan�a uma mensagem de erro e aborta a execu��o do programa
//      line        : Linha do erro
//      errcode     : Nome do tipo de erro
//      message     : Mensagem de erro
//      ...         : Par�metros usados para formata��o
//-----------------------------------------------------------------------------
SHIRO_API void shiro_error(
    const shiro_uint line,
    const shiro_string errcode,
    const shiro_string message,
    ...
) {
    char* err = malloc(1024);

    if (line != 0)
        sprintf(err, "%s on line %lu: %s", errcode, line, message);
    else
        sprintf(err, "%s: %s", errcode, message);

    va_list args;
    va_start(args, message);

    char* error = malloc(1024);
    vsprintf(error, err, args);
    va_end(args);

    last_error = error;

    free(err);
}
//-----------------------------------------------------------------------------
// Obt�m a �ltima mensagem de erro lan�ada pelo shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_string shiro_get_last_error() {
    shiro_string err = last_error;
    last_error = NULL;
    return err;
}
//-----------------------------------------------------------------------------
// Obt�m uma express�o a partir de um token '('
//      stmt    : Ponteiro para o shiro_statement onde est� o token '('
//      offset  : �ndice do token no shiro_statement
//-----------------------------------------------------------------------------
shiro_statement* __expression(
    const shiro_statement* stmt,
    shiro_uint offset,
    shiro_uint sline
) {
    shiro_statement* expression = new_statement(1);

    shiro_uint p_stack = 1, line = 1;
    shiro_token* token = get_token(stmt, offset, &line, sline);

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
// Obt�m um bloco de c�digo a partir de um token '{'
//      stmt    : Ponteiro para o shiro_statement onde est� o token '{'
//      offset  : �ndice do token no shiro_statement
//-----------------------------------------------------------------------------
shiro_statement* __block(
    const shiro_statement* stmt,
    shiro_uint offset,
    shiro_uint sline
) {
    shiro_statement* expression = new_statement(1);

    shiro_uint stack = 1, line = 1;
    shiro_token* token = get_token(stmt, offset, &line, sline);

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
//      binary      : Bin�rio com o operando da esquerda
//      op          : Operador
//      right_hand  : Bin�rio com o operando da direita
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
// Compila uma express�o
//      statement   : Express�o
//      cline       : Linha at� onde o c�digo foi compilado
//-----------------------------------------------------------------------------
shiro_binary* __compile_statement(
    const shiro_statement* statement,
    shiro_uint* line
) {
    shiro_uint sline = *line;

    shiro_binary* binary = new_binary();

    if (statement == NULL || statement->used == 0)
        return binary;

    shiro_uint token_index = 0;

    const shiro_token* token;
    token = get_token(statement, token_index, line, sline);

    // Avan�a uma token
    void next_token() {
        token = get_token(statement, ++token_index, line, sline);
    }

    // Verifica se a token � igual a uma string
    bool token_is(shiro_string tk) {
        if (token == NULL)
            return false;

        return strcmp(token->value, tk) == 0;
    }

    // Obt�m um bin�rio a partir de um bloco
    shiro_binary* parse_block() {
        if (token_is(MARK_OBLOCK)) {
            shiro_statement* block = __block(statement, token_index, sline);

            shiro_protect(
                shiro_binary* b_block = __compile_statements(block, line);
            );

            free_statement(block);

            return b_block;
        } else {
            shiro_error(
                *line,
                ERR_SYNTAX_ERROR,
                "Unexpected '%s', expecting '%s'",
                token->value, MARK_OBLOCK
            );

            return NULL;
        }
    }

    // Obt�m um bin�rio a partir de uma express�o
    shiro_binary* parse_expression() {
        if (token_is(MARK_OEXPR)) {
            shiro_protect(
                shiro_statement* expression = __expression(statement, token_index, sline);
            );

            shiro_protect(
                shiro_binary* b_expr = __compile_statements(expression, line);
            );

            free_statement(expression);

            if (!binary_returns_value(b_expr)) {
                shiro_free_binary(binary);
                shiro_free_binary(b_expr);

                shiro_error(*line, ERR_SYNTAX_ERROR, "Invalid conditional statement");
                return NULL;
            }

            return b_expr;
        } else {
            shiro_error(
                *line,
                ERR_SYNTAX_ERROR,
                "Unexpected '%s', expecting '%s'",
                token->value, MARK_OEXPR
            );
            return NULL;
        }
    }

    if (token == NULL)
        return binary;

    switch (get_token_type(token)) {
        case s_tkKeyword:
        {
            //
            //  exit
            //
            if (strcmp(token->value, KW_DIE) == 0) {
                next_token();

                if (token != NULL && !token_is(MARK_EOS)) {
                    shiro_free_binary(binary);

                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }

                shiro_node* node = new_node(DIE, 0);
                push_node(binary, node);
                free_node(node);
            }

            //
            //  if (<expr>) { <code> } [else { <code> }]
            //
            else if (strcmp(token->value, KW_COND) == 0) {
                shiro_binary *b_expr, *b_block, *b_block_else;

                // Obt�m a express�o condicional e compila
                next_token();
                shiro_protect2(
                    b_expr = parse_expression();
                ,
                    shiro_free_binary(binary);
                );

                // Obt�m o c�digo que ser� executado se a express�o for verdadeira
                next_token();
                shiro_protect2(
                    b_block = parse_block();
                ,
                    shiro_free_binary(b_expr);
                    shiro_free_binary(binary);
                );

                next_token();

                // Se acabou a senten�a, compila e retorna o bin�rio
                if (token == NULL || token_is(MARK_EOS)) {
                    shiro_node* cond = new_node(COND, 0);
                    push_node(b_expr, cond);
                    free_node(cond);

                    shiro_node* jmp = new_node(JUMP, 1, shiro_new_int(b_block->used));
                    push_node(b_expr, jmp);
                    free_node(jmp);

                    binary = concat_and_free_binary(binary, b_expr);
                    binary = concat_and_free_binary(binary, b_block);

                    return binary;
                }

                // Se tiver um else, compila o else
                else if (token_is(KW_ELSE)) {

                    next_token();

                    bool else_if = false;

                    // Se o token for um come�o de bloco de c�digo, compila o
                    // bloco, se for um 'if', compila o if e usa como o bloco
                    if (token_is(MARK_OBLOCK)) {
                        shiro_protect2(
                            b_block_else = parse_block();
                        ,
                            shiro_free_binary(b_expr);
                            shiro_free_binary(b_block);
                            shiro_free_binary(binary);
                        );
                    } else if (token_is(KW_COND)) {
                        shiro_statement* offset = offset_statement(statement, token_index);
                        shiro_protect2(
                            b_block_else = __compile_statement(offset, line);
                        ,
                            shiro_free_binary(b_expr);
                            shiro_free_binary(b_block);
                            shiro_free_binary(binary);
                        );
                        free_statement(offset);

                        else_if = true;
                    } else {
                        shiro_free_binary(b_expr);
                        shiro_free_binary(b_block);
                        shiro_free_binary(binary);

                        if (token != NULL)
                            shiro_error(
                                *line,
                                ERR_SYNTAX_ERROR,
                                "Unexpected '%s', expecting '%s'",
                                token->value, MARK_OBLOCK
                            );
                        else
                            shiro_error(
                                *line,
                                ERR_SYNTAX_ERROR,
                                "Unexpected <END>, expecting '%s'",
                                MARK_OBLOCK
                            );

                        return NULL;
                    }

                    next_token();
                    if (else_if || token == NULL || token_is(MARK_EOS)) {
                        shiro_node* cond = new_node(COND, 0);
                        push_node(b_expr, cond);
                        free_node(cond);

                        shiro_node* jmp0 = new_node(JUMP, 1, shiro_new_int(1));
                        push_node(b_expr, jmp0);
                        free_node(jmp0);

                        shiro_node* jmp1 = new_node(JUMP, 1, shiro_new_int(b_block_else->used + 1));
                        push_node(b_expr, jmp1);
                        free_node(jmp1);

                        b_expr = concat_and_free_binary(b_expr, b_block_else);

                        shiro_node* jmp2 = new_node(JUMP, 1, shiro_new_int(b_block->used));
                        push_node(b_expr, jmp2);
                        free_node(jmp2);

                        binary = concat_and_free_binary(binary, b_expr);
                        binary = concat_and_free_binary(binary, b_block);

                        return binary;
                    } else {
                        shiro_free_binary(b_expr);
                        shiro_free_binary(b_block);
                        shiro_free_binary(b_block_else);
                        shiro_free_binary(binary);

                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }

                } else {
                    shiro_free_binary(b_expr);
                    shiro_free_binary(b_block);
                    shiro_free_binary(binary);

                    shiro_error(
                        *line,
                        ERR_SYNTAX_ERROR,
                        "Unexpected '%s', expecting <END>",
                        token->value
                    );
                    return NULL;
                }
            }

            //
            //  delete <name>
            //
            else if (strcmp(token->value, KW_DELETE) == 0) {
                next_token();

                if (get_token_type(token) == s_tkName) {
                    shiro_string name = token->value;

                    next_token();
                    if (token == NULL || token_is(MARK_EOS)) {
                        shiro_node* del = new_node(FREE, 1, shiro_new_uint(ID(name)));
                        push_node(binary, del);
                        free_node(del);

                        return binary;
                    } else {
                        shiro_free_binary(binary);
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }
                } else {
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                    return NULL;
                }
            }

            //
            //  fn [<name>][([<name>[, <name>[, ...]])] { <code> }
            //
            else if (strcmp(token->value, KW_FUNC) == 0) {
                shiro_string name = NULL;

                next_token();
                if (get_token_type(token) == s_tkName)
                    name = token->value;

                next_token();

                shiro_uint n_args = 0;

                shiro_binary    *bin = new_binary(),
                                *b_args = new_binary(),
                                *b_block;

                // Se tiver argumentos, cria o bin�rio para os argumentos
                if (token_is(MARK_OEXPR)) {
                    shiro_protect2(
                        shiro_statement* args = __expression(statement, token_index, sline);
                    ,
                        shiro_free_binary(bin);
                        shiro_free_binary(b_args);
                        shiro_free_binary(binary);
                    );

                    shiro_uint i;
                    for (i = 0; i < args->used; i += 2, n_args++) {
                        token = get_token(args, i, line, sline);
                        if (token == NULL) {
                            shiro_free_binary(bin);
                            shiro_free_binary(b_args);
                            shiro_free_binary(binary);
                            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting <NAME>");
                            return NULL;
                        } else if (get_token_type(token) == s_tkName) {
                            shiro_string argname = token->value;

                            token = get_token(args, i + 1, line, sline);
                            if (token == NULL || token_is(MARK_LIST)) {
                                shiro_node* arg_push = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ARG(n_args)));
                                push_node(b_args, arg_push);
                                free_node(arg_push);

                                shiro_node* set = new_node(SET_VAR, 1, shiro_new_uint(ID(argname)));
                                push_node(b_args, set);
                                free_node(set);

                                continue;
                            }

                            shiro_free_binary(bin);
                            shiro_free_binary(b_args);
                            shiro_free_binary(binary);
                            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_LIST);
                            return NULL;
                        } else {
                            shiro_free_binary(bin);
                            shiro_free_binary(b_args);
                            shiro_free_binary(binary);
                            shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <NAME>", token->value);
                            return NULL;
                        }
                    }

                    free_statement(args);

                    next_token();
                }

                // Cria o bin�rio para o c�digo da fun��o
                if (token_is(MARK_OBLOCK)) {
                    shiro_protect2(
                        b_block = parse_block();
                    ,
                        shiro_free_binary(bin);
                        shiro_free_binary(b_args);
                        shiro_free_binary(binary);
                    );
                    next_token();

                    if (token == NULL) {
                        bin = concat_and_free_binary(bin, b_args);
                        bin = concat_and_free_binary(bin, b_block);

                        if (!binary_returns_value(bin)) {
                            shiro_node* ret = new_node(PUSH_BY_NAME, 1, ID("nil"));
                            bin = push_node(bin, ret);
                            free_node(ret);

                            ret = new_node(RETURN, 0);
                            bin = push_node(bin, ret);
                            free_node(ret);
                        }

                        shiro_function* fn = shiro_new_fn(n_args, bin);

                        if (name != NULL) {
                            shiro_node* set = new_node(SET_FN, 2, shiro_new_uint(ID(name)), shiro_new_function(fn));
                            push_node(binary, set);
                            free_node(set);
                        } else {
                            shiro_node* push = new_node(PUSH, 1, shiro_new_function(fn));
                            push_node(binary, push);
                            free_node(push);
                        }

                        return binary;
                    } else {
                        shiro_free_binary(bin);
                        shiro_free_binary(b_args);
                        shiro_free_binary(binary);

                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }
                } else {
                    shiro_free_binary(bin);
                    shiro_free_binary(b_args);
                    shiro_free_binary(binary);

                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting '%s'", token->value, MARK_OBLOCK);
                    return NULL;
                }
            }

            //
            //  include "<filename>"
            //
            else if (strcmp(token->value, KW_INCLUDE) == 0) {
                next_token();

                if (token == NULL) {
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting STRING");
                    return NULL;
                }

                if (!token_is(MARK_OEXPR)) {
                    if (get_token_type(token) != s_tkConst ||
                        (*token->value != *MARK_STR1 && *token->value != *MARK_STR2)) {
                        shiro_free_binary(binary);
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting STRING", token->value);
                        return NULL;
                    }

                    shiro_uint len = strlen(token->value) - 2;
                    shiro_string filename = calloc(len + 7, sizeof(shiro_character));
                    memcpy(filename, token->value + 1, len);

                    next_token();

                    if (token == NULL || strcmp(token->value, MARK_EOS) == 0) {

                        if (strrchr(filename, '.') <= strrchr(filename, '/') ||
                            strrchr(filename, '.') <= strrchr(filename, '\\'))
                            strcat(filename, ".shiro");

                        FILE* file = fopen(filename, "r");

                        if (file == NULL) {
                            shiro_free_binary(binary);
                            shiro_error(0, ERR_IO_ERROR, "No such file or directory '%s'", filename);
                            return NULL;
                        }

                        fseek(file, 0, SEEK_END);
                        shiro_uint size = ftell(file);
                        fseek(file, 0, SEEK_SET);

                        shiro_string fcontents = calloc(size, sizeof(shiro_character));
                        fread(fcontents, sizeof(shiro_character), size, file);

                        shiro_protect2(
                            shiro_binary* bin = shiro_compile(fcontents);
                        ,
                            shiro_free_binary(binary);
                        );

                        binary = concat_and_free_binary(binary, bin);

                        return binary;
                    } else {
                        shiro_free_binary(binary);
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }
                } else
                    token = get_token(statement, 0, line, sline);
            }

            //
            //  import "<filename>"
            //
            else if (strcmp(token->value, KW_IMPORT) == 0) {
                next_token();

                if (token == NULL) {
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting STRING");
                    return NULL;
                }

                if (get_token_type(token) == s_tkConst &&
                    (*token->value == *MARK_STR1 || *token->value == *MARK_STR2)) {
                    shiro_uint len = strlen(token->value) - 2;
                    shiro_string filename = calloc(len + 1, sizeof(shiro_character));
                    memcpy(filename, token->value + 1, len);

                    next_token();

                    if (token == NULL || token_is(MARK_EOS)) {
                        shiro_node* push = new_node(PUSH, 1, shiro_new_string(filename));
                        push_node(binary, push);
                        free_node(push);

                        shiro_node* call = new_node(FN_CALL, 2, shiro_new_uint(ID("import")), shiro_new_uint(1));
                        push_node(binary, call);
                        free_node(call);

                        free(filename);

                        return binary;
                    } else {
                        shiro_free_binary(binary);
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }
                }

                token = get_token(statement, 0, line, sline);
            }

            //
            //  require "<filename>"
            //
            else if (strcmp(token->value, KW_REQUIRE) == 0) {
                next_token();

                if (token == NULL) {
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected <END>, expecting STRING");
                    return NULL;
                }

                if (!token_is(MARK_OEXPR)) {
                    if (get_token_type(token) != s_tkConst || (*token->value != *MARK_STR1 && *token->value != *MARK_STR2)) {
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting STRING", token->value);
                        return NULL;
                    }

                    shiro_uint len = strlen(token->value) - 2;
                    shiro_string filename = calloc(len + 7, sizeof(shiro_character));
                    memcpy(filename, token->value + 1, len);

                    next_token();

                    if (token == NULL || strcmp(token->value, MARK_EOS) == 0) {

                        FILE* file = fopen(filename, "rb");

                        if (file == NULL && (strrchr(filename, '.') <= strrchr(filename, '/') ||
                            strrchr(filename, '.') <= strrchr(filename, '\\'))) {
                            strcat(filename, ".iro");
                            file = fopen(filename, "rb");
                        }

                        if (file == NULL) {
                            shiro_error(0, ERR_IO_ERROR, "No such file or directory '%s'", filename);
                            return NULL;
                        }
                        shiro_protect2(
                            shiro_binary* bin = shiro_read_binary(file);
                        ,
                            shiro_free_binary(binary);
                        );

                        binary = concat_and_free_binary(binary, bin);

                        return binary;
                    } else {
                        shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                        return NULL;
                    }
                } else
                    token = get_token(statement, 0, line, sline);
            }

            //
            //  while (<expr>) { <code> }
            //
            else if (strcmp(token->value, KW_WHILE) == 0) {
                shiro_binary *b_expr, *b_block;

                // Compila a express�o condicional
                next_token();
                if (token_is(MARK_OEXPR)) {
                    shiro_protect2(
                        b_expr = parse_expression();
                    ,
                        shiro_free_binary(binary);
                    );
                } else {
                    shiro_free_binary(binary);
                    if (token == NULL)
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected <END>, expecting '%s'",
                            MARK_OEXPR
                        );
                    else
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected '%s', expecting '%s'",
                            token->value, MARK_OEXPR
                        );
                    return NULL;
                }

                // Compila o bloco de c�digo
                next_token();
                if (token_is(MARK_OBLOCK)) {
                    shiro_protect2(
                        b_block = parse_block();
                    ,
                        shiro_free_binary(b_expr);
                        shiro_free_binary(binary);
                    );
                } else {
                    shiro_free_binary(b_expr);
                    shiro_free_binary(binary);
                    if (token == NULL)
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected <END>, expecting '%s'",
                            MARK_OBLOCK
                        );
                    else
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected '%s', expecting '%s'",
                            token->value, MARK_OBLOCK
                        );
                    return NULL;
                }

                // Termina a compila��o
                next_token();
                if (token == NULL || token_is(MARK_EOS)) {
                    concat_binary(binary, b_expr);

                    shiro_node* cond = new_node(COND, 0);
                    push_node(binary, cond);
                    free_node(cond);

                    shiro_node* jmp = new_node(JUMP, 1, shiro_new_int(b_block->used + 1));
                    push_node(binary, jmp);
                    free_node(jmp);

                    concat_binary(binary, b_block);

                    shiro_node* node = new_node(JUMP, 1, shiro_new_int(-(b_expr->used + b_block->used + 3)));
                    push_node(binary, node);
                    free_node(node);

                    shiro_node* ed = new_node(END_LOOP, 0);
                    push_node(binary, ed);
                    free_node(ed);

                    shiro_free_binary(b_expr);
                    shiro_free_binary(b_block);

                    return binary;
                } else {
                    shiro_free_binary(b_expr);
                    shiro_free_binary(b_block);
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }
            }

            //
            //  do { <code> } while (<expr>)
            //
            else if (strcmp(token->value, KW_DO) == 0) {
                shiro_binary *b_expr, *b_block;

                // Compila o bloco de c�digo
                next_token();
                if (token_is(MARK_OBLOCK)) {
                    shiro_protect2(
                        b_block = parse_block();
                    ,
                        shiro_free_binary(binary);
                    );
                } else {
                    shiro_free_binary(binary);
                    if (token == NULL)
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected <END>, expecting '%s'",
                            MARK_OBLOCK
                        );
                    else
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected '%s', expecting '%s'",
                            token->value, MARK_OBLOCK
                        );
                    return NULL;
                }

                // L� o 'while'
                next_token();
                if (!token_is(KW_WHILE)) {
                    shiro_free_binary(b_block);
                    shiro_free_binary(binary);
                    if (token == NULL)
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected <END>, expecting '%s'",
                            KW_WHILE
                        );
                    else
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected '%s', expecting '%s'",
                            token->value, KW_WHILE
                        );
                    return NULL;
                }

                // Compila a express�o condicional
                next_token();
                if (strcmp(token->value, MARK_OEXPR) == 0) {
                    shiro_protect2(
                        b_expr = parse_expression();
                    ,
                        shiro_free_binary(b_block);
                        shiro_free_binary(binary);
                    );
                } else {
                    shiro_free_binary(b_block);
                    shiro_free_binary(binary);
                    if (token == NULL)
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected <END>, expecting '%s'",
                            MARK_OEXPR
                        );
                    else
                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Unexpected '%s', expecting '%s'",
                            token->value, MARK_OEXPR
                        );
                    return NULL;
                }

                // Termina a compila��o
                next_token();
                if (token == NULL || token_is(MARK_EOS)) {
                    concat_binary(binary, b_block);

                    concat_binary(binary, b_expr);
                    shiro_node* cond = new_node(COND, 0);
                    push_node(binary, cond);
                    free_node(cond);

                    shiro_node* jmp = new_node(JUMP, 1, shiro_new_int(1));
                    push_node(binary, jmp);
                    free_node(jmp);

                    shiro_node* node = new_node(JUMP, 1, shiro_new_int(-(b_expr->used + b_block->used + 3)));
                    push_node(binary, node);
                    free_node(node);

                    shiro_node* ed = new_node(END_LOOP, 0);
                    push_node(binary, ed);
                    free_node(ed);

                    shiro_free_binary(b_expr);
                    shiro_free_binary(b_block);

                    return binary;
                } else {
                    shiro_free_binary(b_expr);
                    shiro_free_binary(b_block);
                    shiro_free_binary(binary);
                    shiro_error(
                        *line,
                        ERR_SYNTAX_ERROR,
                        "Unexpected '%s', expecting <END>",
                        token->value
                    );
                    return NULL;
                }
            }

            //
            //  break
            //
            else if (strcmp(token->value, KW_BREAK) == 0) {
                next_token();

                if (token != NULL && !token_is(MARK_EOS)) {
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected expecting '%s', expecting <END>", token->value);
                    return NULL;
                }

                shiro_node* brk = new_node(BREAK, 0);
                push_node(binary, brk);
                free_node(brk);

                return binary;
            }

            //
            //  return <expr>
            //
            else if (strcmp(token->value, KW_RETURN) == 0) {
                next_token();

                if (token == NULL || token_is(MARK_EOS)) {
                    shiro_free_binary(binary);
                    shiro_error(
                        *line,
                        ERR_SYNTAX_ERROR,
                        "Unexpected <END>, expecting value"
                    );
                    return NULL;
                }

                shiro_statement* offset = offset_statement(statement, token_index);
                shiro_binary* b_expr;

                shiro_protect2(
                    b_expr = __compile_statement(offset, line);
                ,
                    shiro_free_binary(binary);
                );

                free_statement(offset);

                if (!binary_returns_value(b_expr)) {
                    shiro_free_binary(binary);
                    shiro_free_binary(b_expr);
                    shiro_error(
                        *line,
                        ERR_SYNTAX_ERROR,
                        "Invalid return statement: returned expression doesn't yield any value"
                    );
                    return NULL;
                }

                binary = concat_and_free_binary(binary, b_expr);

                shiro_node* ret = new_node(RETURN, 0);
                push_node(binary, ret);
                free_node(ret);

                return binary;
            }
        }
        case s_tkName:
        {
            shiro_string name = token->value;

            next_token();

            // Se a senten�a acabou, d� um PUSH_BY_NAME
            if (token == NULL || token_is(MARK_EOS)) {
                shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
                push_node(binary, node);
                free_node(node);

                return binary;
            }

            // Se tiver um operador, compila a opera��o
            else if (get_token_type(token) == s_tkBinaryOperator) {
                shiro_statement* val_stmt = offset_statement(statement, token_index + 1);
                shiro_binary* b_val;

                shiro_protect2(
                    b_val = __compile_statement(val_stmt, line);
                ,
                    shiro_free_binary(binary);
                );
                free_statement(val_stmt);

                // Se for um '=', faz um SET_VAR
                if (token_is(OP_SET)) {

                    if (!binary_returns_value(b_val)) {
                        shiro_free_binary(b_val);
                        shiro_free_binary(binary);

                        shiro_error(
                            *line,
                            ERR_SYNTAX_ERROR,
                            "Invalid variable value: statement doesn't yield any value"
                        );
                        return NULL;
                    }

                    binary = concat_and_free_binary(binary, b_val);

                    shiro_node* set = new_node(SET_VAR, 1, shiro_new_uint(ID(name)));
                    push_node(binary, set);
                    free_node(set);
                }

                // Se for um operador bin�rio seguido de um '=', faz a opera��o
                // junto com um SET_VAR
                else if (*(token->value + 1) == *OP_SET && (
                    *token->value == *OP_ADD || *token->value == *OP_SUB ||
                    *token->value == *OP_MUL || *token->value == *OP_DIV ||
                    *token->value == *OP_MOD || *token->value == *OP_AND ||
                    *token->value == *OP_OR  || *token->value == *OP_XOR)) {

                    shiro_string op = calloc(2, sizeof(shiro_character));
                    *op = *token->value;

                    shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
                    push_node(binary, node);
                    free_node(node);

                    shiro_protect2(
                        binary = __compile_operator(binary, op, b_val, line);
                    ,
                        shiro_free_binary(b_val);
                        shiro_free_binary(binary);
                        free(op);
                    );
                    free(op);

                    shiro_node* set = new_node(SET_VAR, 1, shiro_new_uint(ID(name)));
                    push_node(binary, set);
                    free_node(set);
                }

                // Se for s� um operador normal, faz s� a opera��o normal
                else {
                    shiro_string op = token->value;

                    shiro_node* node = new_node(PUSH_BY_NAME, 1, shiro_new_uint(ID(name)));
                    push_node(binary, node);
                    free_node(node);

                    shiro_protect2(
                        binary = __compile_operator(binary, op, b_val, line);
                    ,
                        shiro_free_binary(b_val);
                        shiro_free_binary(binary);
                    );
                }

                return binary;
            }

            // Se tiver um '.' l� uma propriedade do objeto
            // N�o implementado
            else if (token_is(MARK_PROP)) {
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
            }

            // Se tiver uma express�o com par�ntesis, d� um FN_CALL
            else if (token_is(MARK_OEXPR)) {
                shiro_statement* expression;
                shiro_protect2(
                     expression = __expression(statement, token_index, sline);
                ,
                    shiro_free_binary(binary);
                );
                {
                    shiro_uint n_args = 0;
                    shiro_statement* arg = new_statement(1);

                    shiro_binary* bin = new_binary();

                    int i, l = 0;
                    for (
                        i = 0, token = get_token(expression, i, line, sline);
                        i <= expression->used;
                        token = get_token(expression, ++i, line, sline)
                    ) {
                        if (token == NULL) {
                            free_statement(arg);

                            arg = offset_statement(expression, l);

                            shiro_protect2(
                                shiro_binary* b_arg = __compile_statement(arg, line);
                            ,
                                shiro_free_binary(bin);
                                shiro_free_binary(binary);
                                free_statement(arg);
                            );
                            free_statement(arg);

                            if (!binary_returns_value(b_arg)) {

                                if (n_args == 0) {
                                    shiro_free_binary(b_arg);
                                    break;
                                }

                                shiro_free_binary(bin);
                                shiro_free_binary(binary);
                                free_statement(arg);

                                shiro_error(
                                    *line,
                                    ERR_SYNTAX_ERROR,
                                    "Invalid last argument for function '%s':"
                                    " argument doesn't yield any value",
                                    name
                                );
                                return NULL;
                            }

                            n_args++;
                            bin = concat_and_free_binary(b_arg, bin);

                            break;
                        } else if (token_is(MARK_LIST)) {
                            shiro_token* tk;
                            int j;
                            for (
                                j = l,
                                tk = expression->tokens[j];
                                tk != token && j < expression->used;
                                tk = expression->tokens[++j]
                            )
                                push_token(arg, tk);

                            l = i + 1;

                            shiro_protect2(
                                shiro_binary* b_arg = __compile_statement(arg, line);
                            ,
                                shiro_free_binary(bin);
                                shiro_free_binary(binary);
                                free_statement(arg);
                            );

                            if (!binary_returns_value(b_arg)) {
                                shiro_free_binary(bin);
                                shiro_free_binary(binary);
                                free_statement(arg);

                                shiro_error(
                                    *line,
                                    ERR_SYNTAX_ERROR,
                                    "Invalid argument %d for function '%s':"
                                    " argument doesn't yield any value",
                                    n_args + 1, name
                                );
                                return NULL;
                            }

                            free_statement(arg);
                            arg = new_statement(1);
                            n_args++;
                            bin = concat_and_free_binary(b_arg, bin);
                        }
                    }

                    binary = concat_and_free_binary(binary, bin);

                    shiro_node* fcall = new_node(FN_CALL, 2, shiro_new_uint(ID(name)), shiro_new_uint(n_args));
                    push_node(binary, fcall);
                    free_node(fcall);
                }
                free_statement(expression);
                next_token();

                // Se tiver um '.' l� uma propriedade do objeto
                // N�o implementado
                if (token_is(MARK_PROP)) {
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
                }

                // Se tiver um operador, compila a opera��o
                else if (get_token_type(token) == s_tkBinaryOperator) {
                    shiro_statement* rest = offset_statement(statement, token_index + 1);

                    shiro_binary*  b_val;
                    shiro_protect2(
                        b_val = __compile_statement(rest, line);
                    ,
                        free_statement(rest);
                        shiro_free_binary(binary);
                    );
                    free_statement(rest);
                    shiro_string op = token->value;

                    shiro_protect2(
                        __compile_operator(binary, op, b_val, line);
                    ,
                        shiro_free_binary(binary);
                    );

                    return binary;
                } else if (token != NULL && strcmp(token->value, MARK_EOS) != 0) {
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }

                else if (token != NULL && !token_is(MARK_EOS)) {
                    shiro_free_binary(binary);
                    shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                    return NULL;
                }

                return binary;
            }

            else {
                shiro_free_binary(binary);
                shiro_error(*line, ERR_SYNTAX_ERROR, "Unexpected '%s', expecting <END>", token->value);
                return NULL;
            }
        }
        case s_tkMark:
        {
            if (strcmp(token->value, MARK_OEXPR) == 0) {
                shiro_protect(
                    shiro_statement* expr = __expression(statement, 0, sline);
                );
                shiro_protect(
                    shiro_binary* bin = __compile_statement(expr, line);
                );
                binary = concat_and_free_binary(binary, bin);
                free_statement(expr);

                token = get_token(statement, 1, line, sline);
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
                shiro_statement* block = __block(statement, 0, sline);
                shiro_protect(
                    shiro_binary* bin = __compile_statement(block, line);
                );
                binary = concat_and_free_binary(binary, bin);
                free_statement(block);

                token = get_token(statement, 1, line, sline);
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
                    shiro_long l = strtoll(value, &end, 10);

                    if (l < 2147483647L && l > -2147483647L)
                        s_value = shiro_new_int((shiro_int)l);
                    else
                        s_value = shiro_new_long(l);
                }

                shiro_node* node = new_node(PUSH, 1, s_value);
                push_node(binary, node);
                free_node(node);
            }

            token = get_token(statement, 1, line, sline);
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
// Compila o c�digo passado
//      code    : C�digo
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
