#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stdbool.h>
#include <ctype.h>

#ifndef NULL
#define NULL (void*)0
#endif // NULL

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
#define KW_FUNC         "fn"
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

enum __token_type {
    TK_KEYWORD,
    TK_MARK,
    TK_B_OPERATOR,
    TK_COMPARATOR,
    TK_U_OPERATOR,
    TK_WHITESPACE,
    TK_CONST,
    TK_NAME,
    TK_UNKNOWN
};

typedef enum __token_type burn_token_type;

bool            is_operator       (const char*);
bool            is_operator_c     (const char);
bool            is_symbol         (const char);
bool            is_whitespace     (const char);
char*           append_to_string  (char*, unsigned int*, unsigned int*, char c);
char*           clear_token       (char*, unsigned int*, unsigned int*);
char*           push_token        (char*, unsigned int*, unsigned int*, const char*);
burn_token_type get_token_type    (const char*);

unsigned int shiro_tokenize(const char*, char*);


#endif // LEXER_H_INCLUDED
