//=============================================================================
// src\eval.c
//-----------------------------------------------------------------------------
// Define as funções usadas para executar código Shiro compilado
//=============================================================================
#include "eval.h"
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
                    (value->type == s_tInt &&
                    value_get_field(value, ID("__value"))->value.i != 0) ||
                    (value->type == s_tString &&
                     value_get_field(value, ID("length"))->value.i != 0))
                    i++;

                stack_drop_value(runtime);
                break;
            }
            case JUMP:
            {
                i += value_get_field(node->args[0], ID("__value"))->value.i;
                break;
            }
            case FN_CALL:
            {
                shiro_id id = value_get_field(node->args[0], ID("__value"))->value.u;

                const shiro_function* f = get_global(runtime, id)->value.func;

                if (f->type == s_fnShiroBinary)
                    shiro_execute(runtime, f->s_binary);
                else
                    stack_push_value(runtime, (*f->native)(runtime, value_get_field(node->args[1], ID("__value"))->value.u));

                break;
            }
            case PUSH:
                stack_push_value(runtime, node->args[0]);
                break;
            default:
                break;
        }
    }
    return runtime;
}
//shiro_runtime* shiro_eval(shiro_runtime*, shiro_string);
