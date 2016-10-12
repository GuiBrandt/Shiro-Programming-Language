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
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "vm.h"

#ifdef __DEBUG__
#include <windows.h> // PARA BENCHMARK, APAGAR DEPOIS!
#endif // __DEBUG__

//=============================================================================
// Constantes
//-----------------------------------------------------------------------------
// Esses valores são usados pelo compilador para definir o tipo do um token e
// convertê-lo em código compilado
//=============================================================================
#define DF_TOKEN_SIZE   32

#define KW_COND         "if"
#define KW_ELSE         "else"
#define KW_PRIVATE      "private"
#define KW_PROTECTED    "protected"
#define KW_PUBLIC       "public"
#define KW_SWITCH       "switch"
#define KW_CASE         "case"
#define KW_FOR          "for"
#define KW_DO           "do"
#define KW_WHILE        "while"
#define KW_LOOP         "loop"
#define KW_BREAK        "break"
#define KW_NEXT         "continue"
#define KW_DELETE       "delete"
#define KW_FUNC         "function"
#define KW_PROC         "procedure"
#define KW_VAR          "var"
#define KW_CONST        "const"
#define KW_CLASS        "class"

#define MARK_SEPP       ":"
#define MARK_COND       "?"
#define MARK_STR1       "\""
#define MARK_STR2       "'"
#define MARK_OBLOCK     "{"
#define MARK_CBLOCK     "}"
#define MARK_OEXPR      "("
#define MARK_CEXPR      ")"
#define MARK_LIST       ","
#define MARK_PROP       "."
#define MARK_EOS        ";"
#define MARK_EOL        "\n"

#define OP_SET          "="
#define OP_ADD          "+"
#define OP_SUB          "-"
#define OP_MUL          "*"
#define OP_DIV          "/"
#define OP_MOD          "%"
#define OP_AND          "&"
#define OP_OR           "|"
#define OP_XOR          "^"
#define OP_ASSOC        "=>"

#define CMP_EQU         "=="
#define CMP_GRT         ">"
#define CMP_GRTEQU      ">="
#define CMP_LT          "<"
#define CMP_LTEQU       "<="
#define CMP_DIF         "!="

#define U_NOT           "!"
#define U_INC           "++"
#define U_DEC           "--"
#define U_B_NOT         "~"

#define WS_SPACE        ' '
#define WS_CARET_RETURN '\r'
#define WS_TAB          '\t'
//=============================================================================
//  Declaração
//-----------------------------------------------------------------------------
// As funções que são definidas nesse arquivo devem primeiro ser declaradas
// aqui
//=============================================================================
bool __is_operator       (const char*);
bool __is_operator_c     (char);
bool __is_symbol         (char);
bool __is_whitespace     (char);
void __append_to_string  (char**, unsigned int*, unsigned int*, char c);
void __clear_token       (char**, unsigned int*, unsigned int*);
void __push_token        (char**, unsigned int*, unsigned int*, const char*);
void __error             (unsigned int, const char*, const char*, ...);
void __compile_statement (const char*);

typedef enum __tokentype {
    TK_KEYWORD,
    TK_MARK,
    TK_B_OPERATOR,
    TK_COMPARATOR,
    TK_U_OPERATOR,
    TK_WHITESPACE,
    TK_CONST,
    TK_NAME
} burn_token_type;

burn_token_type __get_token_type(const char*);
//=============================================================================
//  Implementação
//-----------------------------------------------------------------------------
// Daqui pra baixo começam umas tretas fortes com ponteiros. Não se arrisque
// demais mexendo nisso.
//=============================================================================
//---------------------------------------------------------------------------
// Verifica se um caractere é um espaço em branco
//      c   : Caractere a verificar
//---------------------------------------------------------------------------
bool __is_whitespace(const char c) {
    return c == WS_SPACE || c == WS_TAB || c == WS_CARET_RETURN;
}
//-----------------------------------------------------------------------------
// Verifica se um caractere é um símbolo
//      c   : Caractere a verificar
//-----------------------------------------------------------------------------
bool __is_symbol(const char c) {
    bool result = c == *MARK_EOL    || c == *MARK_EOS    || c == *MARK_SEPP  ||
                  c == *MARK_COND   || c == *MARK_OEXPR  || c == *MARK_CEXPR ||
                  c == *MARK_OBLOCK || c == *MARK_CBLOCK || c == *MARK_LIST  ||
                  c == *MARK_PROP;
    if (result) return true;

    char* str = (char*)malloc(2);
    str[0] = c;
    str[1] = 0;
    bool r = __is_operator(str);
    free(str);
    return r;
}
//-----------------------------------------------------------------------------
// Verifica se um char é um operador válido
//      c   : Caractere a verificar
//-----------------------------------------------------------------------------
bool __is_operator_c(const char c) {
    return  c == *OP_SET || c == *OP_ADD  || c == *OP_SUB || c == *OP_MUL ||
            c == *OP_DIV || c == *OP_MOD  || c == *OP_AND || c == *OP_OR  ||
            c == *OP_XOR || c == *CMP_GRT || c == *CMP_LT || c == *U_NOT  ||
            c == *U_B_NOT;
}
//-----------------------------------------------------------------------------
// Verifica se uma string é um operador válido
//      str   : String a verificar
//-----------------------------------------------------------------------------
bool __is_operator(const char* str) {
    if (str[0] == 0) return false;
    if (str[1] == 0) return __is_operator_c(str[0]);
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
void __append_to_string(
        char** string,
        unsigned int* str_size,
        unsigned int* last,
        char c
) {
    if (*last >= *str_size) {
        char* buf = (char*)malloc(*last);
        memset(*string, 0, *last);
        memcpy(buf, *string, *str_size);
        free(*string);
        (*str_size) *= 2;
        *string = (char*)malloc(*str_size);
        memset(*string, 0, *str_size);
        memcpy(*string, buf, *last);
        free(buf);
    }
    (*string)[(*last)++] = c;
}
//-----------------------------------------------------------------------------
// Esvazia uma string liberando memória
//      string  : String a esvaziar
//      size    : Ponteiro para o tamanho da string
//      index   : Pointeiro para o índice do último caractere da string
//-----------------------------------------------------------------------------
void __clear_token(char** string, unsigned int* size, unsigned int* index) {
    free(*string);
    *size = DF_TOKEN_SIZE;
    *index = 0;
    *string = (char*)malloc(*size);
    memset(*string, 0, *size);
}
//-----------------------------------------------------------------------------
// Adiciona uma string a uma lista de tokens
//      tokens          : Lista de tokens
//      tokens_size     : Tamanho do buffer alocado para a lista
//      last            : Índice do último elemento da lista
//      string          : String a ser adicionada à lista
//-----------------------------------------------------------------------------
void __push_token(
        char** tokens,
        unsigned int* tokens_size,
        unsigned int* last,
        const char* string
) {
    int len = strlen(string);

    if (len == 0)
        return;

    if ((*last) + len >= (*tokens_size) - 2) {
        char* buf = (char*)malloc((*last) + len);
        memset(buf, 0, (*last) + len);
        memcpy(buf, *tokens, *tokens_size);
        *tokens_size = (*last) + len + 2;
        free(*tokens);
        *tokens = (char*)malloc(*tokens_size);
        memset(*tokens, 0, *tokens_size);
        memcpy(*tokens, buf, (*last) + len);
        free(buf);
    }

    memcpy((*tokens) + (*last), string, len);
    *last += len + 1;
}
//-----------------------------------------------------------------------------
// Lança uma mensagem de erro
//      errcode     : Nome do tipo de erro
//      message     : Mensagem de erro
//-----------------------------------------------------------------------------
void __error(unsigned int line, const char* errcode, const char* message, ...) {
    char* err = (char*)malloc(1024);
    sprintf(err, "%s on line %d: %s", errcode, line, message);

    va_list args;
    va_start(args, message);
    vfprintf(stderr, err, args);
    va_end(args);
}
//-----------------------------------------------------------------------------
// Separa código em tokens
//      code    : Código para ser separado em tokens
//      out     : Ponteiro para um ponteiro de lista de tokens
//-----------------------------------------------------------------------------
unsigned int burn_tokenize(const char* code, char* out) {
    unsigned int
        size = strlen(code),
        i,
        l = 0, l_sz = DF_TOKEN_SIZE,
        t = 0, t_sz = size * 2;

    char* tokens = (char*)malloc(t_sz + 2);
    memset(tokens, 0, t_sz + 2);

    char* lexeme = (char*)malloc(l_sz);
    memset(lexeme, 0, l_sz);

    char c;

    for (i = 0; i < size; i++) {
        c = code[i];

        if (__is_whitespace(c)) {
            __push_token(&tokens, &t_sz, &t, lexeme);
            __clear_token(&lexeme, &l_sz, &l);
        } else if (__is_symbol(c)) {
            __push_token(&tokens, &t_sz, &t, lexeme);

            bool op2 = false;
            if (size > i + 1) {
                char* str = (char*)malloc(3);
                str[0] = c;
                str[1] = code[i + 1];
                str[2] = 0;

                if (__is_operator(str)) {
                    __push_token(&tokens, &t_sz, &t, str);
                    i++;
                    op2 = true;
                }

                free(str);
            }

            if (!op2) {
                char* str = (char*)malloc(2);
                str[0] = c;
                str[1] = 0;
                __push_token(&tokens, &t_sz, &t, str);
                free(str);
            }

            __clear_token(&lexeme, &l_sz, &l);
        } else if (c == *MARK_STR1 || c == *MARK_STR2) {
            __push_token(&tokens, &t_sz, &t, lexeme);
            __clear_token(&lexeme, &l_sz, &l);

            char s = c;

            do {
                __append_to_string(&lexeme, &l_sz, &l, c);
            } while ((c = code[++i]) != s && i < size);
            __append_to_string(&lexeme, &l_sz, &l, c);

            __push_token(&tokens, &t_sz, &t, lexeme);
            __clear_token(&lexeme, &l_sz, &l);
        } else
            __append_to_string(&lexeme, &l_sz, &l, c);
    }

    if (lexeme[0] != 0)
        __push_token(&tokens, &t_sz, &t, lexeme);

    free(lexeme);
    memcpy(out, tokens, t_sz);
    free(tokens);

    return t_sz;
}
//-----------------------------------------------------------------------------
// Retorna o tipo de uma token
//      Token   : Token a analisar
//-----------------------------------------------------------------------------
burn_token_type __get_token_type(const char* token) {

    if (strcmp(token, KW_BREAK)     == 0 || strcmp(token, KW_CASE)    == 0 ||
        strcmp(token, KW_CLASS)     == 0 || strcmp(token, KW_COND)    == 0 ||
        strcmp(token, KW_CONST)     == 0 || strcmp(token, KW_DELETE)  == 0 ||
        strcmp(token, KW_ELSE)      == 0 || strcmp(token, KW_FOR)     == 0 ||
        strcmp(token, KW_WHILE)     == 0 || strcmp(token, KW_LOOP)    == 0 ||
        strcmp(token, KW_FUNC)      == 0 || strcmp(token, KW_PROC)    == 0 ||
        strcmp(token, KW_DO)        == 0 || strcmp(token, KW_PRIVATE) == 0 ||
        strcmp(token, KW_PROTECTED) == 0 || strcmp(token, KW_PUBLIC)  == 0 ||
        strcmp(token, KW_SWITCH)    == 0 || strcmp(token, KW_NEXT)    == 0)
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

    /*typedef enum __tokentype {
        TK_KEYWORD,
        TK_MARK,
        TK_B_OPERATOR,
        TK_COMPARATOR,
        TK_U_OPERATOR,
        TK_WHITESPACE,
        TK_CONST,
        TK_NAME
    } burn_token_type;*/

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
// Compila uma expressão
//      statement   : Expressão
//-----------------------------------------------------------------------------
void __compile_statement(const char* statement) {

    if (*statement == 0)
        return;

    int line;

    const char* __token(int n) {
        line = 1;

        int i, j;
        for (i = j = 0; j < n && (statement[i] + statement[i + 1]) != 0; i++, j++)
            while (statement[i] != 0) {
                if (statement[i] == '\n')
                    line++;
                i++;
            }

        if ((statement[i] + statement[i - 1]) == 0)
            return NULL;
        return statement + i;
    }

    const char* token = __token(0);

    switch (__get_token_type(token)) {
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


            break;
        case TK_NAME:
            break;
        case TK_MARK:
            break;
        case TK_CONST:
            break;
        default:
            __error(line, "UNEXPECTED_TOKEN", "Unexpected token '%s'", token);
            break;
    }
}
//-----------------------------------------------------------------------------
// Compila o código passado
//      code    : Código
//-----------------------------------------------------------------------------
void burn_compile(const char* code) {
    char* tokens = (char*)malloc(strlen(code) * 2 + 1);
    memset(tokens, 0, strlen(code) * 2 + 1);
    burn_tokenize(code, tokens);

    unsigned int i = 0, t_sz = DF_TOKEN_SIZE, t = 0;
    char* token = (char*)malloc(t_sz);
    memset(token, 0, t_sz);

    unsigned int s_sz = strlen(code) / 2 + 1, s = 0;
    char* statement = (char*)malloc(s_sz);
    memset(statement, 0, s_sz);

    unsigned int line = 1,
                 stack = 0,
                 p_stack = 0;

    void __process_token() {
        if (strcmp(token, MARK_EOL) == 0) {
            line++;
            return;
        } else if (strcmp(token, MARK_OBLOCK) == 0)
            if (p_stack != 0) {
                __error(line, "CANT_OPEN_BLOCK", "Unexpected '%s', expecting '%s'", MARK_OBLOCK, MARK_CEXPR);
                return;
            } else
                stack++;
        else if (strcmp(token, MARK_CBLOCK) == 0) {
            if (stack > 0)
                stack--;
            else {
                __error(line, "CANT_CLOSE_BLOCK", "Unexpected '%s'", MARK_CBLOCK);
                return;
            }
        } else if (strcmp(token, MARK_OEXPR) == 0)
            p_stack++;
        else if (strcmp(token, MARK_CEXPR) == 0) {
            if (p_stack > 0)
                p_stack--;
            else {
                __error(line, "CANT_CLOSE_EXPR", "Unexpected '%s'", MARK_CEXPR);
                return;
            }
        }

        __push_token(&statement, &s_sz, &s, token);

        if (strcmp(token, MARK_EOS) == 0 && stack == 0 && p_stack == 0) {
            __compile_statement(statement);
            free(statement);
            statement = (char*)malloc(s_sz);
            memset(statement, 0, s_sz);
            s = 0;
        }
    }

    for (; (tokens[i] + tokens[i + 1]) != 0; i++) {
        if (tokens[i] == 0) {
            __process_token();
            __clear_token(&token, &t_sz, &t);
        } else
            __append_to_string(&token, &t_sz, &t, tokens[i]);
    }
    free(tokens);

    __process_token();
    free(token);

    if (stack > 0)
        __error(line, "BLOCK_NOT_CLOSED", "A code block was not closed");

    __compile_statement(statement);

    free(statement);
}
//-----------------------------------------------------------------------------
// Ponto de entrada para teste
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
    static const char* code = "<= puts('Hello World');";

    double get_time() {
        LARGE_INTEGER t, f;
        QueryPerformanceCounter(&t);
        QueryPerformanceFrequency(&f);
        return (double)t.QuadPart/(double)f.QuadPart;
    }

    double t0 = get_time();
    burn_compile(code);
    printf("\n\n%f ms\n", (get_time() - t0) * 1000);

    return 0;
}
