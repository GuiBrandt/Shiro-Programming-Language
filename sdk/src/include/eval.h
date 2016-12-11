#ifndef EVAL_H_INCLUDED
#define EVAL_H_INCLUDED

#include "types.h"
#include "parser.h"
#include "vm.h"

SHIRO_API shiro_runtime* shiro_execute              (shiro_runtime*, shiro_binary*);
SHIRO_API shiro_runtime* shiro_execute_for_value    (shiro_runtime*, shiro_value*, shiro_binary*);
SHIRO_API shiro_runtime* shiro_call_function        (const shiro_function*, shiro_runtime*, shiro_uint n_args);

#endif // EVAL_H_INCLUDED
