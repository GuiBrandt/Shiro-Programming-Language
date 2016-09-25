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
} burn_bytecode;

typedef struct node {
    burn_bytecode code;
    void**        args;
} burn_node, *burn_binary;

typedef enum types {
    b_tObject       = 0x00,
    b_tInt          = 0x10,
    b_tFloat        = 0x11,
    b_tString       = 0x20,
    b_tArray        = 0x30,
    b_tFunction     = 0x40,
    b_tVoid         = -1
} burn_type;

#ifdef _INTTYPES_H_
    typedef int32_t burn_fixnum;
    typedef int64_t burn_bignum;
#else
    typedef int burn_fixnum;
    typedef long long burn_bignum;
#endif

typedef double burn_float;

typedef struct bstr {
    size_t length;
    char*  data;
} burn_string;

#define BURN_FIXNUM_SZ sizeof(burn_fixnum)
#define BURN_BIGNUM_SZ sizeof(burn_bignum)
#define BURN_FLOAT_SZ  sizeof(burn_float)
#define BURN_STRING_SZ sizeof(burn_string)

typedef struct value {
    burn_type type;
    union {
        struct value*   val;
        burn_fixnum     i;
        burn_bignum     l;
        burn_float      f;
        burn_string     str;
    } *fields;
    size_t n_fields;
} burn_value;

typedef burn_value (*burn_c_function)(burn_value, int);

typedef struct func {
    burn_type type;
    union {
        burn_c_function native;
        burn_binary     natural;
    };
    int n_args;
} burn_function;

#endif // VM_H_INCLUDED
