#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

#include <types.h>
#include <dll.h>

#define ERR_SYNTAX_ERROR    "SyntaxError"
#define ERR_RUNTIME_ERROR   "RuntimeError"
#define ERR_ARGUMENT_ERROR  "ArgumentError"
#define ERR_NOT_A_FUNCTION  "NotAFunction"
#define ERR_TYPE_ERROR      "TypeError"

shiro_string last_error;

void __error(const shiro_uint, const shiro_string, const shiro_string, ...);

SHIRO_API shiro_string shiro_get_last_error(void);

#define shiro_protect(stuff)    stuff;\
                                {\
                                    shiro_string err = shiro_get_last_error();\
                                    \
                                    if (err != NULL) {\
                                        last_error = err;\
                                        return NULL;\
                                    }\
                                }

#endif // ERRORS_H_INCLUDED
