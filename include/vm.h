#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include "types.h"
#include "parser.h"

typedef enum types {
    s_tObject       = 0x00,
    s_tInt          = 0x10,
    s_tFloat        = 0x11,
    s_tString       = 0x20,
    s_tFunction     = 0x30,
    s_tVoid         = -1
} shiro_type;

typedef struct __value {
    shiro_type type;
    union shiro_field {
        struct __value*     val;
        struct __func*      func;
        shiro_fixnum        i;
        shiro_bignum        l;
        shiro_uint          u;
        shiro_float         f;
        shiro_string        str;
    } *fields;
    shiro_uint n_fields;
} shiro_value;

typedef shiro_value* (*shiro_c_function)(shiro_value, shiro_fixnum);

typedef struct __func {
    shiro_type type;
    enum shiro_function_type {
        s_ftShiroBinary,
        s_ftNative
    } ftype;
    union {
        shiro_c_function native;
        shiro_binary     natural;
    };
    int n_args;
} shiro_function;

shiro_value*    new_value           ();
shiro_value*    new_shiro_string    (const shiro_string);
shiro_value*    new_shiro_fixnum    (const shiro_fixnum);
shiro_value*    new_shiro_bignum    (const shiro_bignum);
shiro_value*    new_shiro_uint      (const shiro_uint);
shiro_value*    new_shiro_float     (const shiro_float);
shiro_value*    new_shiro_function  (const shiro_function);
void            free_value          (shiro_value*);

#endif // VM_H_INCLUDED
