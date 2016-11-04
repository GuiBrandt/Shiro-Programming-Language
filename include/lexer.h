#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "types.h"

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

typedef enum __token_type {
    s_tkKeyword,
    s_tkMark,
    s_tkBinaryOperator,
    s_tkComparator,
    s_tkUnaryOperator,
    s_tkWhitespace,
    s_tkConst,
    s_tkName,
    s_tkUnknown
} shiro_token_type;

typedef struct __token {
    shiro_string    value;
    shiro_uint      allocated;
    shiro_uint      used;
} shiro_token;

typedef struct __statement {
    shiro_token** tokens;
    shiro_uint  allocated;
    shiro_uint  used;
} shiro_statement;

bool                is_operator       (const shiro_string);
bool                is_operator_c     (const shiro_character);
bool                is_symbol         (const shiro_character);
bool                is_whitespace     (const shiro_character);

shiro_token*        new_token         (void);
shiro_token*        clone_token       (const shiro_token*);
shiro_token*        append_to_token   (shiro_token*, const shiro_character c);
shiro_token*        clear_token       (shiro_token*);
shiro_token_type    get_token_type    (const shiro_token*);
void                free_token        (shiro_token*);

shiro_statement*    new_statement     (shiro_uint);
shiro_statement*    push_token        (shiro_statement*, const shiro_token*);
shiro_token*        get_token         (const shiro_statement*, const shiro_uint, shiro_uint*);
void                free_statement    (shiro_statement*);

shiro_statement*    shiro_tokenize    (const shiro_string);


#endif // LEXER_H_INCLUDED
