#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

typedef enum types {
    s_tObject       = 0x00,
    s_tInt          = 0x10,
    s_tFloat        = 0x11,
    s_tString       = 0x20,
    s_tArray        = 0x30,
    s_tMethod       = 0x40,
    s_tVoid         = -1
} shiro_type;

typedef int32_t shiro_fixnum;
typedef int64_t shiro_bignum;
typedef size_t  shiro_uint;

typedef double shiro_float;

typedef char  shiro_character;
typedef char* shiro_string;

#endif // TYPES_H_INCLUDED
