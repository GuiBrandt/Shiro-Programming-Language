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
SHIRO_API shiro_runtime* shiro_execute(
    shiro_runtime*  runtime,
    shiro_binary*   binary
) {
    shiro_uint i;
    for (i = 0; i < binary->used; i++) {
        shiro_node* node = binary->nodes[i];

        switch (node->code) {
            case COND:
            {
                shiro_value* value = shiro_get_value(runtime);

                if (value->n_fields == 0 ||
                    (value->type == s_tInt && get_fixnum(value) != 0) ||
                    (value->type == s_tString &&
                     shiro_get_field(value, ID("length"))->value.i != 0))
                    i++;

                shiro_free_value(value);
                shiro_drop_value(runtime);
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

                shiro_field* global = shiro_get_global(runtime, id);

                if (global == NULL || global->type != s_fFunction)
                    __error(0, ERR_NOT_A_FUNCTION, "Calling non-function value of ID %d", id);

                const shiro_function* f = global->value.func;
                shiro_uint n_args = get_uint(node->args[1]);

                if (n_args != f->n_args)
                    __error(0, ERR_ARGUMENT_ERROR, "Wrong number of arguments: expected %d, got %d", f->n_args, n_args);

                shiro_value* returned;
                if (f->type == s_fnShiroBinary) {
                    shiro_value* func_scope = shiro_clone_value(runtime->self);

                    shiro_uint i;
                    for (i = 0; i < n_args; i++) {
                        shiro_value* arg = shiro_get_value(runtime);
                        shiro_set_field(func_scope, ARG(n_args - i - 1), s_fValue, (union __field_value)arg);
                        shiro_drop_value(runtime);
                    }

                    shiro_execute_for_value(runtime, func_scope, f->s_binary);
                    returned = shiro_get_value(runtime);

                    shiro_free_value(func_scope);
                } else {
                    returned = (*f->native)(runtime, n_args);
                    int i;
                    for (i = 0; i < n_args; i++) {
                        shiro_free_value(shiro_get_value(runtime));
                        shiro_drop_value(runtime);
                    }
                }

                shiro_push_value(runtime, returned);
                break;
            }
            case PUSH:
            {
                shiro_push_value(runtime, shiro_clone_value(node->args[0]));
                break;
            }
            case PUSH_BY_NAME:
            {

                shiro_id id = get_uint(node->args[0]);

                if (id == ID("self")) {
                    shiro_push_value(runtime, shiro_clone_value(runtime->self));
                } else if (id == ID("nil")) {
                    shiro_push_value(runtime, shiro_nil);
                } else {
                    shiro_field* g = shiro_get_global(runtime, id);
                    shiro_push_value(runtime, g == NULL ? shiro_nil : shiro_clone_value(g->value.val));
                }
                break;
            }
            case DROP:
                shiro_drop_value(runtime);
                break;
            case ALLOC:
            {
                shiro_set_global(
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
                shiro_field* g = shiro_get_global(runtime, id);

                if (g != NULL)
                    shiro_free_field(g);

                if (node->code == SET_VAR) {
                    shiro_set_global(
                        runtime,
                        id,
                        s_fValue,
                        (union __field_value)shiro_get_value(runtime)
                    );
                    shiro_drop_value(runtime);
                } else
                    shiro_set_global(
                        runtime,
                        id,
                        s_fFunction,
                        (union __field_value)shiro_clone_function(get_func(node->args[1]))
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
                        shiro_free_field(scope->fields[i]);
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
//-----------------------------------------------------------------------------
// * Executa código shiro em um runtime usando um binário pré-compilado
//   definindo o valor de 'self'
//      runtime : Runtime usado para executar o programa
//      value   : Valor que será usado para self
//      binary  : Código Shiro compilado
//-----------------------------------------------------------------------------
SHIRO_API shiro_runtime* shiro_execute_for_value(
    shiro_runtime*  runtime,
    shiro_value*    value,
    shiro_binary*   binary) {

    shiro_value* self = runtime->self;
    runtime->self = value;
    shiro_execute(runtime, binary);
    runtime->self = self;

    return runtime;
}
