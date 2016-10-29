#include "lexer.h"

#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
// Verifica se um caractere é um espaço em branco
//      c   : Caractere a verificar
//---------------------------------------------------------------------------
bool is_whitespace(const char c) {
    return c == WS_SPACE || c == WS_TAB || c == WS_CARET_RETURN;
}
//-----------------------------------------------------------------------------
// Verifica se um caractere é um símbolo
//      c   : Caractere a verificar
//-----------------------------------------------------------------------------
bool is_symbol(const char c) {
    bool result = c == *MARK_EOL    || c == *MARK_EOS    || c == *MARK_SEPP  ||
                  c == *MARK_COND   || c == *MARK_OEXPR  || c == *MARK_CEXPR ||
                  c == *MARK_OBLOCK || c == *MARK_CBLOCK || c == *MARK_LIST  ||
                  c == *MARK_PROP;
    if (result) return true;

    char* str = (char*)malloc(2);
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
bool is_operator_c(const char c) {
    return  c == *OP_SET || c == *OP_ADD  || c == *OP_SUB || c == *OP_MUL ||
            c == *OP_DIV || c == *OP_MOD  || c == *OP_AND || c == *OP_OR  ||
            c == *OP_XOR || c == *CMP_GRT || c == *CMP_LT || c == *U_NOT  ||
            c == *U_B_NOT;
}
//-----------------------------------------------------------------------------
// Verifica se uma string é um operador válido
//      str   : String a verificar
//-----------------------------------------------------------------------------
bool is_operator(const char* str) {
    if (str[0] == 0) return false;
    if (str[1] == 0) return is_operator_c(str[0]);
    else
        return  strcmp(CMP_EQU, str)   == 0 || strcmp(CMP_GRTEQU, str) == 0 ||
                strcmp(CMP_LTEQU, str) == 0 || strcmp(CMP_DIF, str)    == 0 ||
                strcmp(OP_ASSOC, str)  == 0 || strcmp(U_INC, str)      == 0 ||
                strcmp(U_DEC, str)     == 0 || (str[1] == *OP_SET && (
                    str[0] == *OP_ADD || str[0] == *OP_SUB ||
                    str[0] == *OP_MUL || str[0] == *OP_DIV ||
                    str[0] == *OP_MOD || str[0] == *OP_AND ||
                    str[0] == *OP_OR  || str[0] == *OP_XOR));
}
//-----------------------------------------------------------------------------
// Adiciona um caractere ao fim de uma string
//      string      : String a verificar
//      str_size    : Ponteiro para o tamanho do buffer alocado para a string
//      last        : Ponteiro para o índice do último caractere adicionado à
//                    string
//      c           : Caractere a adicionar
//-----------------------------------------------------------------------------
char* append_to_string(
        char* string,
        unsigned int* str_size,
        unsigned int* last,
        char c
) {
    if (*last >= *str_size) {
        (*str_size) *= 2;
        string = realloc(string, *str_size);
    }
    string[(*last)++] = c;
    string[*last] = 0;

    return string;
}
//-----------------------------------------------------------------------------
// Esvazia uma string liberando memória
//      string  : String a esvaziar
//      size    : Ponteiro para o tamanho da string
//      index   : Pointeiro para o índice do último caractere da string
//-----------------------------------------------------------------------------
char* clear_token(char* string, unsigned int* size, unsigned int* index) {
    free(string);
    *size = DF_TOKEN_SIZE;
    *index = 0;
    string = malloc(*size);
    memset(string, 0, *size);
    return string;
}
//-----------------------------------------------------------------------------
// Adiciona uma string a uma lista de tokens
//      tokens          : Lista de tokens
//      tokens_size     : Tamanho do buffer alocado para a lista
//      last            : Índice do último elemento da lista
//      string          : String a ser adicionada à lista
//-----------------------------------------------------------------------------
char* push_token(
        char* tokens,
        unsigned int* tokens_size,
        unsigned int* last,
        const char* string
) {
    int len = strlen(string);

    if (len == 0)
        return tokens;

    if ((*last) + len >= (*tokens_size) - 2) {
        *tokens_size = ((*last) + len) * 2 + 2;
        tokens = realloc(tokens, *tokens_size);
    }

    memcpy(tokens + (*last), string, len);
    *last += len + 1;
    tokens[*last] = tokens[*last + 1] = 0;

    return tokens;
}
//-----------------------------------------------------------------------------
// Retorna o tipo de uma token
//      token   : Token a analisar
//-----------------------------------------------------------------------------
burn_token_type get_token_type(const char* token) {

    if (token == NULL)
        return TK_UNKNOWN;

    if (strcmp(token, KW_BREAK)     == 0 || strcmp(token, KW_CASE)    == 0 ||
        strcmp(token, KW_CLASS)     == 0 || strcmp(token, KW_COND)    == 0 ||
        strcmp(token, KW_CONST)     == 0 || strcmp(token, KW_DELETE)  == 0 ||
        strcmp(token, KW_ELSE)      == 0 || strcmp(token, KW_FOR)     == 0 ||
        strcmp(token, KW_WHILE)     == 0 || strcmp(token, KW_LOOP)    == 0 ||
        strcmp(token, KW_FUNC)      == 0 || strcmp(token, KW_SWITCH)  == 0 ||
        strcmp(token, KW_DO)        == 0 || strcmp(token, KW_PRIVATE) == 0 ||
        strcmp(token, KW_PROTECTED) == 0 || strcmp(token, KW_PUBLIC)  == 0 ||
        strcmp(token, KW_NEXT)      == 0)
        return TK_KEYWORD;

    const char c = *token;

    if (c == *OP_SET || c == *OP_ADD || c == *OP_SUB || c == *OP_MUL ||
        c == *OP_DIV || c == *OP_MOD || c == *OP_AND || c == *OP_OR  ||
        c == *OP_XOR || strcmp(OP_ASSOC, token) == 0 ||
        (token[1] == *OP_SET && (
            token[0] == *OP_ADD || token[0] == *OP_SUB ||
            token[0] == *OP_MUL || token[0] == *OP_DIV ||
            token[0] == *OP_MOD || token[0] == *OP_AND ||
            token[0] == *OP_OR  || token[0] == *OP_XOR)
        ))
        return TK_B_OPERATOR;

    if (strcmp(token, CMP_EQU)    == 0 || strcmp(token, CMP_DIF)   == 0 ||
        strcmp(token, CMP_GRT)    == 0 || strcmp(token, CMP_LT)    == 0 ||
        strcmp(token, CMP_GRTEQU) == 0 || strcmp(token, CMP_LTEQU) == 0)
        return TK_COMPARATOR;

    if (strcmp(U_INC, token) == 0 || strcmp(U_DEC, token) == 0 ||
        c == *U_NOT || c == *U_B_NOT)
        return TK_U_OPERATOR;

    if (isdigit(c) || c == *MARK_STR1 || c == *MARK_STR2)
        return TK_CONST;

    return TK_NAME;
}
//-----------------------------------------------------------------------------
// Separa código em tokens
//      code    : Código para ser separado em tokens
//      out     : Ponteiro para um ponteiro de lista de tokens
//-----------------------------------------------------------------------------
unsigned int shiro_tokenize(const char* code, char* out) {
    unsigned int
        size = strlen(code),
        i,
        l = 0, l_sz = DF_TOKEN_SIZE,
        t = 0, t_sz = size * 2;

    char* tokens = malloc(t_sz + 2);
    memset(tokens, 0, t_sz + 2);

    char* lexeme = malloc(l_sz);
    memset(lexeme, 0, l_sz);

    char c;

    for (i = 0; i < size; i++) {
        c = code[i];

        if (is_whitespace(c)) {
            tokens = push_token(tokens, &t_sz, &t, lexeme);
            lexeme = clear_token(lexeme, &l_sz, &l);
        } else if (is_symbol(c)) {
            tokens = push_token(tokens, &t_sz, &t, lexeme);

            bool op2 = false;
            if (size > i + 1) {
                char* str = malloc(3);
                str[0] = c;
                str[1] = code[i + 1];
                str[2] = 0;

                if (is_operator(str)) {
                    tokens = push_token(tokens, &t_sz, &t, str);
                    i++;
                    op2 = true;
                }

                free(str);
            }

            if (!op2) {
                char* str = malloc(2);
                str[0] = c;
                str[1] = 0;
                tokens = push_token(tokens, &t_sz, &t, str);
                free(str);
            }

            lexeme = clear_token(lexeme, &l_sz, &l);
        } else if (c == *MARK_STR1 || c == *MARK_STR2) {
            tokens = push_token(tokens, &t_sz, &t, lexeme);
            lexeme = clear_token(lexeme, &l_sz, &l);

            char s = c;

            do {
                lexeme = append_to_string(lexeme, &l_sz, &l, c);
            } while ((c = code[++i]) != s && i < size);
            lexeme = append_to_string(lexeme, &l_sz, &l, c);

            tokens = push_token(tokens, &t_sz, &t, lexeme);
            lexeme = clear_token(lexeme, &l_sz, &l);
        } else
            lexeme = append_to_string(lexeme, &l_sz, &l, c);
    }

    if (lexeme[0] != 0)
        tokens = push_token(tokens, &t_sz, &t, lexeme);

    free(lexeme);
    memcpy(out, tokens, t_sz);
    free(tokens);

    return t_sz;
}
