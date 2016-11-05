//=============================================================================
// src\eval.c
//-----------------------------------------------------------------------------
// Define as fun��es usadas para executar c�digo Shiro compilado
//=============================================================================
#include "eval.h"
#include "errors.h"
//-----------------------------------------------------------------------------
// * Executa c�digo shiro em um runtime usando um bin�rio pr�-compilado
//      runtime : Runtime usado para executar o programa
//      binary  : C�digo Shiro compilado
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

                if (global->type != s_fFunction)
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
                        set_value_field(func_scope, ARG(i), s_fValue, (union __field_value)stack_get_value(runtime));
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
                    for (i = 0; i < n_args; i++)
                        stack_drop_value(runtime);
                }

                stack_push_value(runtime, returned);
                break;
            }
            case PUSH:
                stack_push_value(runtime, node->args[0]);
                break;
            case PUSH_BY_NAME:
                stack_push_value(runtime, get_global(runtime, get_uint(node->args[0]))->value.val);
                break;
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

                if (node->code == SET_VAR)
                    set_global(
                        runtime,
                        id,
                        s_fValue,
                        (union __field_value)stack_get_value(runtime)
                    );
                else
                    set_global(
                        runtime,
                        id,
                        s_fFunction,
                        (union __field_value)get_func(node->args[1])
                    );
                break;
            }
            case FREE:
            {
                free_field(get_global(runtime, get_uint(node->args[0])));
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
//shiro_runtime* shiro_eval(shiro_runtime*, shiro_string);
