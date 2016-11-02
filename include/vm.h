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

typedef shiro_uint shiro_id;

typedef struct __field {
    const shiro_id id;
    enum __field_type {
        s_fValue       = 0x00,
        s_fFixnum      = 0x10,
        s_fBignum      = 0x11,
        s_fUInt        = 0x12,
        s_fFloat       = 0x20,
        s_fString      = 0x30,
        s_fFunction    = 0x40
    } type;
    union __field_value {
        struct __value*         val;
        const struct __func*    func;
        shiro_fixnum            i;
        shiro_bignum            l;
        shiro_uint              u;
        shiro_float             f;
        shiro_string            str;
    } value;
} shiro_field;

shiro_id __shiro_parse_field_id_from_name(const shiro_string);
#define ID(str) __shiro_parse_field_id_from_name(str)

typedef struct __value {
    const shiro_type    type;
    shiro_uint          n_fields;
    shiro_field**       fields;
} shiro_value;

typedef shiro_value* (*shiro_c_function)(shiro_value, shiro_fixnum);

typedef struct __func {
    enum shiro_function_type {
        s_fnShiroBinary,
        s_fnNative
    } type;
    union {
        shiro_c_function native;
        shiro_binary*    s_binary;
    };
    int n_args;
} shiro_function;

shiro_field*    clone_field         (shiro_field*);
void            free_field          (shiro_field*);

shiro_value*    new_value           ();
shiro_value*    clone_value         (const shiro_value*);
shiro_field*    value_get_field     (const shiro_value*, const shiro_id, shiro_uint*);
shiro_value*    value_set_field     (shiro_value*, const shiro_field*);
shiro_value*    set_value_field     (shiro_value*, const shiro_id, enum __field_type, const union __field_value);
shiro_value*    new_shiro_string    (const shiro_string);
shiro_value*    new_shiro_fixnum    (const shiro_fixnum);
shiro_value*    new_shiro_bignum    (const shiro_bignum);
shiro_value*    new_shiro_uint      (const shiro_uint);
shiro_value*    new_shiro_float     (const shiro_float);
shiro_value*    new_shiro_function  (const shiro_function*);
void            free_value          (shiro_value*);

void            free_function       (shiro_function*);

#endif // VM_H_INCLUDED
