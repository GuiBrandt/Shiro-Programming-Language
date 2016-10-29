#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include <inttypes.h>

#include "parser.h"

typedef enum types {
    s_tObject       = 0x00,
    s_tInt          = 0x10,
    s_tFloat        = 0x11,
    s_tString       = 0x20,
    s_tArray        = 0x30,
    s_tMethod       = 0x40,
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

typedef char* shiro_string;

#define SHIRO_FIXNUM_SZ sizeof(shiro_fixnum)
#define SHIRO_BIGNUM_SZ sizeof(shiro_bignum)
#define SHIRO_FLOAT_SZ  sizeof(shiro_float)
#define SHIRO_STRING_SZ sizeof(shiro_string)

typedef struct value {
    shiro_type type;
    union shiro_field {
        struct value*    val;
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
