#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

#define ERR_SYNTAX_ERROR    "SyntaxError"
#define ERR_RUNTIME_ERROR   "RuntimeError"
#define ERR_ARGUMENT_ERROR  "ArgumentError"

void __error(const shiro_uint, const shiro_string, const shiro_string, ...);

#endif // ERRORS_H_INCLUDED
