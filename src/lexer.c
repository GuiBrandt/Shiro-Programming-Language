//=============================================================================
// src\lexer.c
//-----------------------------------------------------------------------------
// Define as funções usadas para separar strings em sentenças
//=============================================================================
#include "lexer.h"

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
//---------------------------------------------------------------------------
// Verifica se um caractere é um espaço em branco
//      c   : Caractere a verificar
//---------------------------------------------------------------------------
bool is_whitespace(const shiro_character c) {
    return c == WS_SPACE || c == WS_TAB || c == WS_CARET_RETURN;
}
//-----------------------------------------------------------------------------
// Verifica se um caractere é um símbolo
//      c   : Caractere a verificar
//-----------------------------------------------------------------------------
bool is_symbol(const shiro_character c) {
    bool result = c == *MARK_EOL    || c == *MARK_EOS    || c == *MARK_SEPP  ||
                  c == *MARK_COND   || c == *MARK_OEXPR  || c == *MARK_CEXPR ||
                  c == *MARK_OBLOCK || c == *MARK_CBLOCK || c == *MARK_LIST  ||
                  c == *MARK_PROP;
    if (result) return true;

    shiro_string str = malloc(2);
    str[0] = c;
    str[1] = 0;
    bool r = is_operator(str);
    free(str);
    return r;
}
//-----------------------------------------------------------------------------
// Verifica se um char é um operador válido
//      c   : Caractere a verificar
//-----------------------------------------------------------------------------
bool is_operator_c(const shiro_character c) {
    return  c == *OP_SET || c == *OP_ADD  || c == *OP_SUB || c == *OP_MUL ||
            c == *OP_DIV || c == *OP_MOD  || c == *OP_AND || c == *OP_OR  ||
            c == *OP_XOR || c == *CMP_GRT || c == *CMP_LT || c == *U_NOT  ||
            c == *U_B_NOT;
}
//-----------------------------------------------------------------------------
// Verifica se uma string é um operador válido
//      str   : String a verificar
//-----------------------------------------------------------------------------
bool is_operator(const shiro_string str) {
    if (str[0] == 0) return false;
    if (str[1] == 0) return is_operator_c(str[0]);
    else
        return  strcmp(CMP_EQU, str)   == 0 || strcmp(CMP_GRTEQU, str)   == 0 ||
                strcmp(CMP_LTEQU, str) == 0 || strcmp(CMP_DIF, str)      == 0 ||
                strcmp(OP_ASSOC, str)  == 0 || strcmp(U_INC, str)        == 0 ||
                strcmp(U_DEC, str)     == 0 || strcmp(MARK_COMMENT, str) == 0 ||
                (str[1] == *OP_SET && (
                    str[0] == *OP_ADD || str[0] == *OP_SUB ||
                    str[0] == *OP_MUL || str[0] == *OP_DIV ||
                    str[0] == *OP_MOD || str[0] == *OP_AND ||
                    str[0] == *OP_OR  || str[0] == *OP_XOR));
}
//-----------------------------------------------------------------------------
// Cria uma nova token
//-----------------------------------------------------------------------------
shiro_token* new_token() {
    shiro_token* token = malloc(sizeof(shiro_token));

    token->allocated = DF_TOKEN_SIZE;
    token->used      = 0;
    token->value     = calloc(token->allocated, sizeof(shiro_character));

    return token;
}
//-----------------------------------------------------------------------------
// Retorna uma Deep Copy de uma shiro_token
//-----------------------------------------------------------------------------
shiro_token* clone_token(const shiro_token* other) {
    shiro_token* token = malloc(sizeof(shiro_token));
    token->allocated = other->allocated;
    token->used      = other->used;
    token->value     = calloc(token->allocated, sizeof(shiro_character));
    memcpy(token->value, other->value, other->used);

    return token;
}
//-----------------------------------------------------------------------------
// Adiciona um caractere ao fim de uma token
//      shiro_token : Ponteiro para a estrutura shiro_token que será alterada
//      c           : Caractere a adicionar
//-----------------------------------------------------------------------------
shiro_token* append_to_token(shiro_token* token, const shiro_character c) {
    if (token->used >= token->allocated) {
        token->allocated *= 2;
        token->value = realloc(token->value, token->allocated);
    }

    token->value[token->used++] = c;
    return token;
}
//-----------------------------------------------------------------------------
// Esvazia uma token liberando memória
//      token   : Ponteiro para a shiro_token que será esvaziada
//-----------------------------------------------------------------------------
shiro_token* clear_token(shiro_token* token) {
    token->allocated = DF_TOKEN_SIZE;
    token->used = 0;

    free(token->value);
    token->value = calloc(token->allocated, sizeof(shiro_character));

    return token;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por uma token
//      token   : Token que será liberada da memória
//-----------------------------------------------------------------------------
void free_token(shiro_token* token) {
    free(token->value);
    free(token);
}
//-----------------------------------------------------------------------------
// Cria uma senteça vazia
//      size    : Tamanho inicial da sentença
//-----------------------------------------------------------------------------
shiro_statement* new_statement(size_t size) {
    shiro_statement* stmt = malloc(sizeof(shiro_statement));

    stmt->allocated = size;
    stmt->used      = 0;
    stmt->tokens    = calloc(size, sizeof(shiro_token*));

    return stmt;
}
//-----------------------------------------------------------------------------
// Adiciona uma string a uma sentença
//      statement   : Ponteiro para o shiro_statement onde a token será
//                    adicionada
//      token       : Token que será adicionada
//-----------------------------------------------------------------------------
shiro_statement* push_token(
    shiro_statement* statement,
    const shiro_token* token
) {
    if (statement == NULL || token == NULL ||
        token->value == NULL ||
        *token->value == 0)
        return statement;

    if (statement->used >= statement->allocated) {
        statement->allocated *= 2;
        statement->tokens = realloc(
            statement->tokens,
            statement->allocated * sizeof(shiro_token*)
        );
    }

    statement->tokens[statement->used++] = clone_token(token);
    return statement;
}
//-----------------------------------------------------------------------------
// Obtém uma token em uma determinada posição na sentença, colapsando
// expressões entre parêntesis e blocos de código
//      statement   : Ponteiro para o shiro_statement que contém a token
//      index       : Posição da token
// Retorna NULL se o índice estiver fora do intervalo
//-----------------------------------------------------------------------------
shiro_token* get_token(
    const shiro_statement* statement,
    const shiro_uint index,
    shiro_uint* line
) {
    if (index >= statement->used)
        return NULL;

    *line = 1;
    shiro_uint i, j, p_stack = 0, b_stack = 0;

    for (i = j = 0; (j < index || p_stack != 0 || b_stack != 0) && i < statement->used; i++) {
        if (strcmp(statement->tokens[i]->value, MARK_CEXPR) == 0)
            p_stack--;
        else if (strcmp(statement->tokens[i]->value, MARK_CBLOCK) == 0)
            b_stack--;
        else if (strcmp(statement->tokens[i]->value, MARK_OBLOCK) == 0)
            b_stack++;
        else if (strcmp(statement->tokens[i]->value, MARK_OEXPR) == 0)
            p_stack++;

        if (strcmp(statement->tokens[i]->value, MARK_EOL) == 0)
            (*line)++;
        else if (p_stack == 0 && b_stack == 0)
            j++;
    }

    while (i < statement->used && strcmp(statement->tokens[i]->value, MARK_EOL) == 0) {
        (*line)++;
        i++;
    }

    if (i >= statement->used)
        return NULL;

    return statement->tokens[i];
}
//-----------------------------------------------------------------------------
// Retorna uma cópia de um statement com n tokens removidos do começo
//      statement   : Statement a ser copiado e deslocado
//      off         : Número de tokens a deslocar
//-----------------------------------------------------------------------------
shiro_statement* offset_statement(
    const shiro_statement* statement,
    const shiro_uint off
) {
    shiro_statement* stmt = new_statement(statement->used - off);

    shiro_uint line;
    shiro_token* tk = get_token(statement, off - 1, &line);

    if (tk == NULL)
        return stmt;

    shiro_uint i = 0;
    while (statement->tokens[i] != tk)
        i++;
    i++;

    for (; i < statement->used; i++)
        push_token(stmt, statement->tokens[i]);

    return stmt;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por uma sentença
//      statement   : shiro_statement que será liberado da memória
//-----------------------------------------------------------------------------
void free_statement(shiro_statement* statement) {
    int i;
    for (i = 0; i < statement->used; i++)
        free_token(statement->tokens[i]);
    free(statement->tokens);
    free(statement);
}
//-----------------------------------------------------------------------------
// Retorna o tipo de uma token
//      token   : Token a analisar
//-----------------------------------------------------------------------------
shiro_token_type get_token_type(const shiro_token* token) {

    if (token == NULL)
        return s_tkUnknown;

    const shiro_string string = token->value;

    if (string == NULL || *string == 0)
        return s_tkUnknown;

    if (strcmp(string, KW_BREAK)     == 0 || strcmp(string, KW_CASE)    == 0 ||
        strcmp(string, KW_CLASS)     == 0 || strcmp(string, KW_COND)    == 0 ||
        strcmp(string, KW_CONST)     == 0 || strcmp(string, KW_DELETE)  == 0 ||
        strcmp(string, KW_ELSE)      == 0 || strcmp(string, KW_FOR)     == 0 ||
        strcmp(string, KW_WHILE)     == 0 || strcmp(string, KW_LOOP)    == 0 ||
        strcmp(string, KW_FUNC)      == 0 || strcmp(string, KW_SWITCH)  == 0 ||
        strcmp(string, KW_DO)        == 0 || strcmp(string, KW_PRIVATE) == 0 ||
        strcmp(string, KW_PROTECTED) == 0 || strcmp(string, KW_PUBLIC)  == 0 ||
        strcmp(string, KW_NEXT)      == 0 || strcmp(string, KW_DIE)     == 0 ||
        strcmp(string, KW_VAR)       == 0 || strcmp(string, KW_SELF)    == 0 ||
        strcmp(string, KW_NIL)       == 0)
        return s_tkKeyword;

    const shiro_character c = *string;

    if (c == *MARK_OEXPR    || c == *MARK_CEXPR || c == *MARK_OBLOCK ||
        c == *MARK_CBLOCK   || c == *MARK_EOS   || c == *MARK_PROP   ||
        c == *MARK_SEPP)
        return s_tkMark;

    if (c == *OP_SET || c == *OP_ADD || c == *OP_SUB || c == *OP_MUL ||
        c == *OP_DIV || c == *OP_MOD || c == *OP_AND || c == *OP_OR  ||
        c == *OP_XOR || strcmp(OP_ASSOC, string) == 0 ||
        (string[1] == *OP_SET && (
            string[0] == *OP_ADD || string[0] == *OP_SUB ||
            string[0] == *OP_MUL || string[0] == *OP_DIV ||
            string[0] == *OP_MOD || string[0] == *OP_AND ||
            string[0] == *OP_OR  || string[0] == *OP_XOR)
        ))
        return s_tkBinaryOperator;

    if (strcmp(string, CMP_EQU)    == 0 || strcmp(string, CMP_DIF)   == 0 ||
        strcmp(string, CMP_GRT)    == 0 || strcmp(string, CMP_LT)    == 0 ||
        strcmp(string, CMP_GRTEQU) == 0 || strcmp(string, CMP_LTEQU) == 0)
        return s_tkComparator;

    if (strcmp(U_INC, string) == 0 || strcmp(U_DEC, string) == 0 ||
        c == *U_NOT || c == *U_B_NOT)
        return s_tkUnaryOperator;

    if (isdigit(c) || c == *MARK_STR1 || c == *MARK_STR2)
        return s_tkConst;

    return s_tkName;
}
//-----------------------------------------------------------------------------
// Separa código em tokens
//      code    : Código para ser separado em tokens
//-----------------------------------------------------------------------------
shiro_statement* shiro_tokenize(
    const shiro_string code
) {
    unsigned int size = strlen(code), i;

    shiro_statement* tokens = new_statement((size / DF_TOKEN_SIZE * 2) + 1);
    shiro_token* lexeme = new_token();

    shiro_character c;

    for (i = 0; i < size; i++) {
        c = code[i];

        if (is_whitespace(c)) {
            push_token(tokens, lexeme);
            clear_token(lexeme);
        } else if (is_symbol(c)) {
            push_token(tokens, lexeme);

            bool op2 = false;
            if (size > i + 1) {
                shiro_string str = malloc(3 * sizeof(shiro_character));
                str[0] = c;
                str[1] = code[i + 1];
                str[2] = 0;

                if (is_operator(str)) {
                    shiro_token* tk = malloc(sizeof(shiro_token));
                    tk->allocated = 3;
                    tk->used = 3;
                    tk->value = calloc(3, sizeof(shiro_character));
                    memcpy(tk->value, str, 3);

                    if (strcmp(tk->value, MARK_COMMENT) == 0) {
                        while (i < size && code[i] != *MARK_EOL)
                            i++;
                        i--;
                    } else {
                        push_token(tokens, tk);
                        i++;
                    }
                    free_token(tk);
                    op2 = true;
                }

                free(str);
            }

            if (!op2) {
                char* str = malloc(2);
                str[0] = c;
                str[1] = 0;

                shiro_token* tk = malloc(sizeof(shiro_token));
                tk->allocated = 2;
                tk->used = 2;
                tk->value = calloc(2, sizeof(shiro_character));
                memcpy(tk->value, str, 2);

                tokens = push_token(tokens, tk);
                free_token(tk);
                free(str);
            }

            clear_token(lexeme);
        } else if (c == *MARK_STR1 || c == *MARK_STR2) {
            push_token(tokens, lexeme);
            clear_token(lexeme);

            const shiro_character s = c;
            bool escape = false;

            do {
                if (c == *MARK_ESCAPE && !escape)
                    escape = true;
                else {
                        if (escape) {
                            if (c == 'n')
                                append_to_token(lexeme, '\n');
                            else if (c == 't')
                                append_to_token(lexeme, '\t');
                            else if (c == 'b')
                                append_to_token(lexeme, '\b');
                            else if (c == 'r')
                                append_to_token(lexeme, '\r');
                            else if (c == '0')
                                append_to_token(lexeme, '\0');
                            else
                                append_to_token(lexeme, c);
                        } else
                            append_to_token(lexeme, c);
                        escape = false;
                }
            } while (((c = code[++i]) != s || escape) && i < size);
            append_to_token(lexeme, c);

            push_token(tokens, lexeme);
            clear_token(lexeme);
        } else
            append_to_token(lexeme, c);
    }

    if (lexeme->used > 0)
        push_token(tokens, lexeme);

    free_token(lexeme);

    return tokens;
}
