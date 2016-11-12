#ifndef SHIRO_H_INCLUDED
#define SHIRO_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>

#if defined(_WIN32) && !defined(SHIRO_STATIC)
#   define SHIRO_API __declspec(dllimport)
#else
#   define SHIRO_API
#endif

typedef int32_t shiro_fixnum;
typedef int64_t shiro_bignum;
typedef size_t  shiro_uint;

typedef double shiro_float;

typedef char  shiro_character;
typedef char* shiro_string;

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

SHIRO_API shiro_binary* shiro_compile(const shiro_string);
SHIRO_API void          shiro_free_binary(shiro_binary*);

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
        struct __func*          func;
        shiro_fixnum            i;
        shiro_bignum            l;
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
#define get_fixnum(val) get_value(val).i
#define get_bignum(val) get_value(val).l
#define get_float(val)  get_value(val).f
#define get_string(val) get_value(val).str

typedef struct __value {
    const shiro_type    type;
    shiro_uint          n_fields;
    shiro_field**       fields;
} shiro_value;

typedef struct __runtime {
    shiro_uint      used_stack;
    shiro_uint      allocated_stack;
    shiro_value**   stack;

    shiro_value*    self;
} shiro_runtime;

typedef shiro_value* (*shiro_c_function)(struct __runtime*, shiro_fixnum);

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

#define ARG(n) (shiro_id)(0x7C9432C7 + n)

shiro_field*              shiro_clone_field         (shiro_field*);
void                      shiro_free_field          (shiro_field*);

SHIRO_API shiro_value*    shiro_new_value     ();
SHIRO_API shiro_value*    shiro_clone_value   (const shiro_value*);
SHIRO_API shiro_field*    shiro_get_field     (const shiro_value*, const shiro_id);
SHIRO_API shiro_value*    shiro_def_field     (shiro_value*, const shiro_field*);
SHIRO_API shiro_value*    shiro_set_field     (shiro_value*, const shiro_id, enum __field_type, union __field_value);
SHIRO_API shiro_value*    shiro_new_string    (const shiro_string);
SHIRO_API shiro_value*    shiro_new_fixnum    (const shiro_fixnum);
SHIRO_API shiro_value*    shiro_new_bignum    (const shiro_bignum);
SHIRO_API shiro_value*    shiro_new_uint      (const shiro_uint);
SHIRO_API shiro_value*    shiro_new_float     (const shiro_float);
SHIRO_API shiro_value*    shiro_new_function  (shiro_function*);
SHIRO_API void            shiro_free_value    (shiro_value*);

SHIRO_API shiro_function* shiro_clone_function(const shiro_function*);
SHIRO_API void            shiro_free_function (shiro_function*);

SHIRO_API shiro_runtime*  shiro_init          ();
SHIRO_API void            shiro_terminate     (shiro_runtime*);

SHIRO_API shiro_runtime*  shiro_push_value    (shiro_runtime*, shiro_value*);
SHIRO_API shiro_runtime*  shiro_drop_value    (shiro_runtime*);
SHIRO_API shiro_value*    shiro_get_last_value(shiro_runtime*);
SHIRO_API shiro_value*    shiro_get_value     (shiro_runtime* runtime, shiro_uint n);

SHIRO_API shiro_runtime*  shiro_def_global    (shiro_runtime*, shiro_field*);
SHIRO_API shiro_runtime*  shiro_set_global    (shiro_runtime*, shiro_id, enum __field_type, union __field_value);
SHIRO_API shiro_field*    shiro_get_global    (shiro_runtime*, shiro_id);

SHIRO_API shiro_string    shiro_to_string     (shiro_value*);

SHIRO_API shiro_value* shiro_nil;

SHIRO_API shiro_runtime* shiro_execute(shiro_runtime*, shiro_binary*);
SHIRO_API shiro_runtime* shiro_execute_for_value(shiro_runtime*, shiro_value*, shiro_binary*);

SHIRO_API void shiro_error(const shiro_uint, const shiro_string, const shiro_string, ...);
SHIRO_API shiro_string shiro_get_last_error(void);

#endif // SHIRO_H_INCLUDED
