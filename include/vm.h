#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include "types.h"
#include "parser.h"

typedef struct __value {
    shiro_type type;
    union shiro_field {
        struct __value*  val;
        shiro_fixnum     i;
        shiro_bignum     l;
        shiro_float      f;
        shiro_string     str;
    } *fields;
    size_t n_fields;
} shiro_value;

typedef shiro_value (*shiro_c_function)(shiro_value, int);

typedef struct __func {
    shiro_type type;
    union {
        shiro_c_function native;
        shiro_binary     natural;
    };
    int n_args;
} shiro_function;

#endif // VM_H_INCLUDED
