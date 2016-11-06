//=============================================================================
// src\eval.c
//-----------------------------------------------------------------------------
// Define as funções usadas para executar código Shiro compilado
//=============================================================================
#include "eval.h"
#include "errors.h"

#include <stdlib.h>
//-----------------------------------------------------------------------------
// * Executa código shiro em um runtime usando um binário pré-compilado
//      runtime : Runtime usado para executar o programa
//      binary  : Código Shiro compilado
//-----------------------------------------------------------------------------
shiro_runtime* shiro_execute(shiro_runtime* runtime, shiro_binary* binary) {
    shiro_uint i;
    for (i = 0; i < binary->used; i++) {
        shiro_node* node = binary->nodes[i];

        switch (node->code) {
            case COND:
            {
                shiro_value* value = stack_get_value(runtime);

                if (value->n_fields == 0 ||
                    (value->type == s_tInt && get_fixnum(value) != 0) ||
                    (value->type == s_tString &&
                     value_get_field(value, ID("length"))->value.i != 0))
                    i++;

                free_value(value);
                stack_drop_value(runtime);
                break;
            }
            case JUMP:
            {
                i += get_fixnum(node->args[0]);
                break;
            }
            case FN_CALL:
            {
                shiro_id id = get_uint(node->args[0]);

                shiro_field* global = get_global(runtime, id);

                if (global == NULL || global->type != s_fFunction)
                    __error(0, ERR_NOT_A_FUNCTION, "Calling non-function value of ID %d", id);

                const shiro_function* f = global->value.func;
                shiro_uint n_args = get_uint(node->args[1]);

                if (n_args != f->n_args)
                    __error(0, ERR_ARGUMENT_ERROR, "Wrong number of arguments: expected %d, got %d", f->n_args, n_args);

                shiro_value* returned;
                if (f->type == s_fnShiroBinary) {
                    shiro_value* self = runtime->self;

                    shiro_value* func_scope = clone_value(self);

                    shiro_uint i;
                    for (i = 0; i < n_args; i++) {
                        shiro_value* arg = stack_get_value(runtime);
                        set_value_field(func_scope, ARG(n_args - i - 1), s_fValue, (union __field_value)arg);
                        stack_drop_value(runtime);
                    }

                    runtime->self = func_scope;
                    shiro_execute(runtime, f->s_binary);
                    returned = stack_get_value(runtime);
                    runtime->self = self;

                    free_value(func_scope);
                } else {
                    returned = (*f->native)(runtime, n_args);
                    int i;
                    for (i = 0; i < n_args; i++) {
                        free_value(stack_get_value(runtime));
                        stack_drop_value(runtime);
                    }
                }

                stack_push_value(runtime, returned);
                break;
            }
            case PUSH:
            {
                stack_push_value(runtime, clone_value(node->args[0]));
                break;
            }
            case PUSH_BY_NAME:
            {
                shiro_field* g = get_global(runtime, get_uint(node->args[0]));
                stack_push_value(runtime, g == NULL ? shiro_nil : clone_value(g->value.val));
                break;
            }
            case DROP:
                stack_drop_value(runtime);
                break;
            case ALLOC:
            {
                set_global(
                    runtime,
                    get_uint(node->args[0]),
                    s_fValue,
                    (union __field_value)shiro_nil
                );
                break;
            }
            case SET_VAR:
            case SET_FN:
            {
                shiro_id id = get_uint(node->args[0]);
                shiro_field* g = get_global(runtime, id);

                if (g != NULL)
                    free_field(g);

                if (node->code == SET_VAR) {
                    set_global(
                        runtime,
                        id,
                        s_fValue,
                        (union __field_value)stack_get_value(runtime)
                    );
                    stack_drop_value(runtime);
                } else
                    set_global(
                        runtime,
                        id,
                        s_fFunction,
                        (union __field_value)clone_function(get_func(node->args[1]))
                    );
                break;
            }
            case FREE:
            {
                shiro_id id = get_uint(node->args[0]);

                shiro_value* scope = runtime->self;

                shiro_uint i;
                for (i = 0; i < scope->n_fields; i++)
                    if (scope->fields[i] != NULL && scope->fields[i]->id == id) {
                        free_field(scope->fields[i]);
                        scope->fields[i] = NULL;
                    }
                break;
            }
            case DIE:
                return runtime;
            default:
                break;
        }
    }
    return runtime;
}
