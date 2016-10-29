#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

typedef enum bytecodes {
    VAR             = 0x01,
    FN              = 0x02,
    CLASS           = 0x03,

    COND            = 0x04,
    JUMP            = 0x05,
    FN_CALL         = 0x06,

    ALLOC           = 0x10,
    ALLOC_VAR       = ALLOC | VAR,
    ALLOC_FN        = ALLOC | FN,
    ALLOC_CLASS     = ALLOC | CLASS,

    FREE            = 0x20,
    FREE_VAR        = FREE | VAR,
    FREE_FN         = FREE | FN,
    FREE_CLASS      = FREE | CLASS,

    PUSH            = 0x30,
    DROP            = 0x31,

    FINISH          = 0xED,
} shiro_bytecode;

typedef struct node {
    shiro_bytecode code;
    void**         args;
    unsigned int   n_args;
} shiro_node;

typedef struct binary {
    shiro_node* nodes;
    unsigned int size;
} shiro_binary;

shiro_binary shiro_compile(const char*);

#endif // PARSER_H_INCLUDED
