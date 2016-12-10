#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

#include <types.h>
#include <dll.h>

#define ERR_SYNTAX_ERROR    "SyntaxError"
#define ERR_RUNTIME_ERROR   "RuntimeError"
#define ERR_ARGUMENT_ERROR  "ArgumentError"
#define ERR_NOT_A_FUNCTION  "NotAFunction"
#define ERR_TYPE_ERROR      "TypeError"
#define ERR_IO_ERROR        "IOError"

shiro_string last_error;

SHIRO_API void shiro_error(const shiro_uint, const shiro_string, const shiro_string, ...);

SHIRO_API shiro_string shiro_get_last_error(void);

#define shiro_protect(crit)     crit;\
                                {\
                                    if (last_error != NULL)\
                                        return NULL;\
                                }

#define shiro_protect2(crit, save)  crit;\
                                    {\
                                        if (last_error != NULL) {\
                                            save;\
                                            return NULL;\
                                        }\
                                    }

#endif // ERRORS_H_INCLUDED
