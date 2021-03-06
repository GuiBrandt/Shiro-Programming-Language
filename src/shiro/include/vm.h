#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include "types.h"
#include "parser.h"
#include "dll.h"

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
        s_fInteger     = 0x10,
        s_fLong        = 0x11,
        s_fUInt        = 0x12,
        s_fFloat       = 0x20,
        s_fString      = 0x30,
        s_fFunction    = 0x40
    } type;
    union __field_value {
        struct __value*         val;
        struct __func*          func;
        shiro_int               i;
        shiro_long              l;
        shiro_uint              u;
        shiro_float             f;
        shiro_string            str;
    } value;
} shiro_field;

SHIRO_API shiro_id shiro_parse_id_from_name(const shiro_string);
#define ID(str) shiro_parse_id_from_name(str)

#define ID_VALUE ID("__value")

#define get_value(val)  shiro_get_field(val, ID_VALUE)->value
#define get_func(val)   get_value(val).func
#define get_uint(val)   get_value(val).u
#define get_int(val) get_value(val).i
#define get_long(val) get_value(val).l
#define get_float(val)  get_value(val).f
#define get_string(val) get_value(val).str

typedef struct __value {
    const shiro_type    type;
    shiro_uint          n_fields;
    shiro_field**       fields;
    shiro_uint          being_used;
} shiro_value;

typedef struct __runtime {
    shiro_uint      used_stack;
    shiro_uint      allocated_stack;
    shiro_value**   stack;

    shiro_value*    self;
} shiro_runtime;

typedef shiro_value* (*shiro_c_function)(struct __runtime*, shiro_int);

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
    shiro_uint being_used;
} shiro_function;

#define ARG(n) (shiro_id)(0x7C9432C7 + n)

shiro_field*              shiro_clone_field     (shiro_field*);
void                      shiro_free_field      (shiro_field*);

SHIRO_API shiro_value*    shiro_new_value       ();
SHIRO_API shiro_value*    shiro_use_value       (shiro_value*);
SHIRO_API shiro_value*    shiro_clone_value     (const shiro_value*);
SHIRO_API shiro_field*    shiro_get_field       (const shiro_value*, const shiro_id);
SHIRO_API shiro_value*    shiro_def_field       (shiro_value*, const shiro_field*);
SHIRO_API shiro_value*    shiro_set_field       (shiro_value*, const shiro_id, enum __field_type, union __field_value);
SHIRO_API shiro_value*    shiro_new_string      (const shiro_string);
SHIRO_API shiro_value*    shiro_new_int      (const shiro_int);
SHIRO_API shiro_value*    shiro_new_long        (const shiro_long);
SHIRO_API shiro_value*    shiro_new_uint        (const shiro_uint);
SHIRO_API shiro_value*    shiro_new_float       (const shiro_float);
SHIRO_API shiro_value*    shiro_new_function    (shiro_function*);
SHIRO_API void            shiro_free_value      (shiro_value*);

SHIRO_API shiro_function* shiro_new_native      (shiro_uint, shiro_c_function);
SHIRO_API shiro_function* shiro_new_fn          (shiro_uint, shiro_binary*);
SHIRO_API shiro_function* shiro_use_function    (shiro_function*);
SHIRO_API void            shiro_free_function   (shiro_function*);

SHIRO_API shiro_runtime*  shiro_init            ();
SHIRO_API void            shiro_terminate       (shiro_runtime*);

SHIRO_API shiro_runtime*  shiro_push_value      (shiro_runtime*, shiro_value*);
SHIRO_API shiro_runtime*  shiro_drop_value      (shiro_runtime*);
SHIRO_API shiro_value*    shiro_get_last_value  (shiro_runtime*);
SHIRO_API shiro_value*    shiro_get_value       (shiro_runtime* runtime, shiro_uint n);

SHIRO_API shiro_runtime*  shiro_def_global      (shiro_runtime*, shiro_field*);
SHIRO_API shiro_runtime*  shiro_set_global      (shiro_runtime*, shiro_id, enum __field_type, union __field_value);
SHIRO_API shiro_field*    shiro_get_global      (shiro_runtime*, shiro_id);

SHIRO_API bool            shiro_to_bool         (shiro_value*);
SHIRO_API shiro_string    shiro_to_string       (shiro_value*);
SHIRO_API shiro_int       shiro_to_int          (shiro_value*);
SHIRO_API shiro_long      shiro_to_long         (shiro_value*);
SHIRO_API shiro_uint      shiro_to_uint         (shiro_value*);
SHIRO_API shiro_float     shiro_to_float        (shiro_value*);

#define   shiro_nil       ((shiro_value*)-1)

SHIRO_API void            shiro_load_stdlib     (shiro_runtime*);

#define __native_fn_name(name) __shiro_imported_ ## name

#define shiro_native(name) shiro_value* __native_fn_name(name) (shiro_runtime* runtime, shiro_uint n_args)
#define shiro_def_native(runtime, name, n_args) shiro_set_global(runtime, ID(#name), s_fFunction, (union __field_value)shiro_new_native(n_args, (shiro_c_function)&__native_fn_name(name)));
#define shiro_call_native(runtime, name, n_args) __native_fn_name(name)(runtime, n_args)

#endif // VM_H_INCLUDED
