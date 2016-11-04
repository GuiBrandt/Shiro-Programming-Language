#ifndef EVAL_H_INCLUDED
#define EVAL_H_INCLUDED

#include "types.h"
#include "parser.h"
#include "vm.h"

shiro_runtime* shiro_execute(shiro_runtime*, shiro_binary*);
shiro_runtime* shiro_execute_for_value(shiro_runtime*, shiro_value*, shiro_binary*);
//shiro_runtime* shiro_eval(shiro_runtime*, shiro_string);

#endif // EVAL_H_INCLUDED
