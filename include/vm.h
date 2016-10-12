#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include <inttypes.h>

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
    void**        args;
} shiro_node, *shiro_binary;

typedef enum types {
    s_tObject       = 0x00,
    s_tInt          = 0x10,
    s_tFloat        = 0x11,
    s_tString       = 0x20,
    s_tArray        = 0x30,
    s_tFunction     = 0x40,
    s_tVoid         = -1
} shiro_type;

#ifdef _INTTYPES_H_
    typedef int32_t shiro_fixnum;
    typedef int64_t shiro_bignum;
#else
    typedef int shiro_fixnum;
    typedef long long shiro_bignum;
#endif

typedef double shiro_float;

typedef struct sstr {
    size_t length;
    char*  data;
} shiro_string;

#define SHIRO_FIXNUM_SZ sizeof(shiro_fixnum)
#define SHIRO_BIGNUM_SZ sizeof(shiro_bignum)
#define SHIRO_FLOAT_SZ  sizeof(shiro_float)
#define SHIRO_STRING_SZ sizeof(shiro_string)

typedef struct value {
    shiro_type type;
    union {
        struct value*   val;
        shiro_fixnum     i;
        shiro_bignum     l;
        shiro_float      f;
        shiro_string     str;
    } *fields;
    size_t n_fields;
} shiro_value;

typedef shiro_value (*shiro_c_function)(shiro_value, int);

typedef struct func {
    shiro_type type;
    union {
        shiro_c_function native;
        shiro_binary     natural;
    };
    int n_args;
} shiro_function;

#endif // VM_H_INCLUDED
