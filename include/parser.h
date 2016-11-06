#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "types.h"
#include "lexer.h"

typedef enum bytecodes {
    COND            = 0x04,
    JUMP            = 0x05,
    FN_CALL         = 0x06,

    VAR             = 0x01,
    FN              = 0x02,
    CLASS           = 0x04,

    ALLOC           = 0x10,

    SET             = 0x20,
    SET_VAR         = SET | VAR,
    SET_FN          = SET | FN,
    SET_CLASS       = SET | CLASS,

    PUSH            = 0x30,
    PUSH_BY_NAME    = 0x31,
    DROP            = 0x32,

    FREE            = 0x50,

    COMPARE         = 0x40,
    GT              = 0x01,
    LT              = 0x02,
    EQ              = 0x04,
    COMPARE_GT      = COMPARE | GT,
    COMPARE_LT      = COMPARE | LT,
    COMPARE_EQ      = COMPARE | EQ,
    COMPARE_LT_EQ   = COMPARE | LT | EQ,
    COMPARE_GT_EQ   = COMPARE | GT | EQ,

    DIE             = 0xED,
} shiro_bytecode;

typedef struct __node {
    shiro_bytecode      code;
    struct __value**    args;
    shiro_uint          n_args;
} shiro_node;

typedef struct __binary {
    shiro_node** nodes;
    shiro_uint   allocated;
    shiro_uint   used;
} shiro_binary;

shiro_node*   new_node                  (const shiro_bytecode, const shiro_uint, ...);
shiro_node*   clone_node                (const shiro_node*);
void          free_node                 (shiro_node*);

shiro_binary* new_binary                (void);
shiro_binary* clone_binary              (const shiro_binary*);
shiro_binary* push_node                 (shiro_binary*, const shiro_node*);
shiro_binary* concat_binary             (shiro_binary*, const shiro_binary*);
bool          binary_returns_value      (const shiro_binary*);
void          free_binary               (shiro_binary*);
shiro_binary* concat_and_free_binary    (shiro_binary*, shiro_binary*);

shiro_binary* shiro_compile             (const shiro_string);

#endif // PARSER_H_INCLUDED
