//=============================================================================
// src\eval.c
//-----------------------------------------------------------------------------
// Define as funções usadas para executar código Shiro compilado
//=============================================================================
#include "eval.h"
#include "errors.h"

#include <stdlib.h>
#include <string.h>
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
                shiro_value* value = shiro_get_last_value(runtime);

                if (!shiro_to_bool(value))
                    i++;

                shiro_drop_value(runtime);
                break;
            }
            case JUMP:
            {
                i += get_int(node->args[0]);
                break;
            }
            case FN_CALL:
            {
                shiro_id id = get_uint(node->args[0]);

                shiro_field* global = shiro_get_global(runtime, id);

                if (global == NULL) {
                    shiro_error(0, ERR_NOT_A_FUNCTION, "nil is not a function");
                    return NULL;
                }

                if (global->type == s_fValue)
                    global = shiro_get_field(global->value.val, ID_VALUE);

                if (global->type != s_fFunction) {
                    shiro_error(0, ERR_NOT_A_FUNCTION, "Calling non-function value of ID %d", id);
                    return NULL;
                }

                const shiro_function* f = global->value.func;
                shiro_uint n_args = get_uint(node->args[1]);

                if (shiro_call_function(f, runtime, n_args) == NULL)
                    return NULL;
                break;
            }
            case PUSH:
            {
                shiro_push_value(runtime, shiro_use_value(node->args[0]));
                break;
            }
            case PUSH_BY_NAME:
            {
                shiro_id id = get_uint(node->args[0]);

                if (id == ID("self")) {
                    shiro_push_value(runtime, shiro_use_value(runtime->self));
                } else if (id == ID("nil")) {
                    shiro_push_value(runtime, shiro_nil);
                } else {
                    shiro_field* g = shiro_get_global(runtime, id);

                    if (g == NULL)
                        shiro_push_value(runtime, shiro_nil);
                    else
                        switch (g->type) {
                            case s_fValue:
                                shiro_push_value(runtime, shiro_use_value(g->value.val));
                                break;
                            case s_fLong:
                                shiro_push_value(runtime, shiro_new_long(g->value.l));
                                break;
                            case s_fInteger:
                                shiro_push_value(runtime, shiro_new_int(g->value.i));
                                break;
                            case s_fUInt:
                                shiro_push_value(runtime, shiro_new_uint(g->value.u));
                                break;
                            case s_fFloat:
                                shiro_push_value(runtime, shiro_new_float(g->value.f));
                                break;
                            case s_fString:
                                shiro_push_value(runtime, shiro_new_string(g->value.str));
                                break;
                            case s_fFunction:
                                shiro_push_value(runtime, shiro_new_function(g->value.func));
                                break;
                        }
                }
                break;
            }
            case DROP:
                shiro_drop_value(runtime);
                break;
            case SET_VAR:
            case SET_FN:
            {
                shiro_id id = get_uint(node->args[0]);

                if (node->code == SET_VAR) {
                    shiro_set_global(
                        runtime,
                        id,
                        s_fValue,
                        (union __field_value)shiro_use_value(shiro_get_last_value(runtime))
                    );
                    shiro_drop_value(runtime);
                } else
                    shiro_set_global(
                        runtime,
                        id,
                        s_fFunction,
                        (union __field_value)shiro_use_function(get_func(node->args[1]))
                    );

                break;
            }
            case ADD:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fString: {
                        shiro_string stringified = shiro_to_string(r);
                        shiro_uint len = shiro_get_field(l, ID("length"))->value.u,
                                    l = strlen(stringified);

                        shiro_string str = calloc(l + len + 1, sizeof(shiro_character));
                        memcpy(str, l_v->value.str, len);
                        memcpy(str + len, stringified, l);

                        result = shiro_new_string(str);
                        shiro_set_field(result, ID_VALUE, s_fString, (union __field_value)str);
                        shiro_set_field(result, ID("length"), s_fInteger, (union __field_value)(len + l));
                        break;
                    }
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i + intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l + longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u + longified);
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = shiro_new_float(l_v->value.f + floatified);
                        break;
                    }
                    case s_fFunction: {
                        if (r_v->type != s_fFunction) {
                            shiro_error(0, ERR_TYPE_ERROR, "Incompatible types");
                            return NULL;
                        }
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }
                shiro_free_value(l);
                shiro_free_value(r);
                shiro_push_value(runtime, result);

                break;
            }
            case SUB:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL ||
                    l_v->type == s_fString ||
                    l_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL ||
                    r_v->type == s_fString ||
                    r_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i - intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l - longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u - longified);
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = shiro_new_float(l_v->value.f - floatified);
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }
                shiro_free_value(l);
                shiro_free_value(r);
                shiro_push_value(runtime, result);

                break;
            }
            case MUL:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL ||
                    l_v->type == s_fString ||
                    l_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL ||
                    r_v->type == s_fString ||
                    r_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i * intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l * longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u * longified);
                        break;
                    }
                    case s_fFloat: {
                        shiro_int floatified = shiro_to_float(r);
                        result = shiro_new_float(l_v->value.f * floatified);
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }
                shiro_free_value(l);
                shiro_free_value(r);
                shiro_push_value(runtime, result);

                break;
            }
            case DIV:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL ||
                    l_v->type == s_fString ||
                    l_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL ||
                    r_v->type == s_fString ||
                    r_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i / intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l / longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u / longified);
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = shiro_new_float(l_v->value.f / floatified);
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }
                shiro_free_value(l);
                shiro_free_value(r);
                shiro_push_value(runtime, result);

                break;
            }
            case MOD:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL ||
                    l_v->type == s_fString ||
                    l_v->type == s_fFunction ||
                    l_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL ||
                    r_v->type == s_fString ||
                    r_v->type == s_fFunction ||
                    l_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i % intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l % longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u % longified);
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }
                shiro_free_value(l);
                shiro_free_value(r);
                shiro_push_value(runtime, result);

                break;
            }
            case B_AND:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL ||
                    l_v->type == s_fString ||
                    l_v->type == s_fFunction ||
                    r_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL ||
                    r_v->type == s_fString ||
                    r_v->type == s_fFunction ||
                    r_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i & intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l & longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u & longified);
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }
                shiro_free_value(l);
                shiro_free_value(r);
                shiro_push_value(runtime, result);

                break;
            }
            case B_OR:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL ||
                    l_v->type == s_fString ||
                    l_v->type == s_fFunction ||
                    r_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL ||
                    r_v->type == s_fString ||
                    r_v->type == s_fFunction ||
                    r_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i | intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l | longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u | longified);
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }

                shiro_free_value(l);
                shiro_free_value(r);

                shiro_push_value(runtime, result);

                break;
            }
            case B_XOR:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL ||
                    l_v->type == s_fString ||
                    l_v->type == s_fFunction ||
                    r_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL ||
                    r_v->type == s_fString ||
                    r_v->type == s_fFunction ||
                    r_v->type == s_fFloat) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                shiro_value* result;

                switch (l_v->type) {
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = shiro_new_int(l_v->value.i ^ intified);
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = shiro_new_long(l_v->value.l ^ longified);
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = shiro_new_uint(l_v->value.u ^ longified);
                        break;
                    }
                    default:
                        result = shiro_new_value();
                        break;
                }

                shiro_free_value(l);
                shiro_free_value(r);

                shiro_push_value(runtime, result);

                break;
            }
            case COMPARE_EQ:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                bool result = false;

                switch (l_v->type) {
                    case s_fString: {
                        shiro_string stringified = shiro_to_string(r);
                        result = strcmp(l_v->value.str, stringified) == 0;
                        free(stringified);
                        break;
                    }
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = l_v->value.i == intified;
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = l_v->value.l == longified;
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = l_v->value.u == longified;
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = l_v->value.f == floatified;
                        break;
                    }
                    default:
                        result = l_v == r_v;
                        break;
                }

                shiro_push_value(runtime, shiro_new_int(result));

                shiro_free_value(r);
                shiro_free_value(l);
                break;
            }
            case COMPARE_GT:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL || l_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL || r_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                bool result = false;

                switch (l_v->type) {
                    case s_fString: {
                        shiro_string stringified = shiro_to_string(r);
                        result = strcmp(l_v->value.str, stringified) > 0;
                        free(stringified);
                        break;
                    }
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = l_v->value.i > intified;
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = l_v->value.l > longified;
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = l_v->value.u > longified;
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = l_v->value.f > floatified;
                        break;
                    }
                    default:
                        break;
                }

                shiro_push_value(runtime, shiro_new_int(result));

                shiro_free_value(r);
                shiro_free_value(l);
                break;
            }
            case COMPARE_LT:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL || l_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL || r_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                bool result = false;

                switch (l_v->type) {
                    case s_fString: {
                        shiro_string stringified = shiro_to_string(r);
                        result = strcmp(l_v->value.str, stringified) < 0;
                        free(stringified);
                        break;
                    }
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = l_v->value.i < intified;
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = l_v->value.l < longified;
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = l_v->value.u < longified;
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = l_v->value.f < floatified;
                        break;
                    }
                    default:
                        break;
                }

                shiro_push_value(runtime, shiro_new_int(result));

                shiro_free_value(r);
                shiro_free_value(l);
                break;
            }
            case COMPARE_GT_EQ:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL || l_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL || r_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                bool result = false;

                switch (l_v->type) {
                    case s_fString: {
                        shiro_string stringified = shiro_to_string(r);
                        result = strcmp(l_v->value.str, stringified) >= 0;
                        free(stringified);
                        break;
                    }
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = l_v->value.i >= intified;
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = l_v->value.l >= longified;
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = l_v->value.u >= longified;
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = l_v->value.f >= floatified;
                        break;
                    }
                    default:
                        break;
                }

                shiro_push_value(runtime, shiro_new_int(result));

                shiro_free_value(r);
                shiro_free_value(l);
                break;
            }
            case COMPARE_LT_EQ:
            {
                shiro_value* r = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_value* l = shiro_use_value(shiro_get_last_value(runtime));
                shiro_drop_value(runtime);

                shiro_field* r_v = shiro_get_field(r, ID_VALUE);
                shiro_field* l_v = shiro_get_field(l, ID_VALUE);

                if (l_v == NULL || l_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid left-hand operand");
                    return NULL;
                }

                if (r_v == NULL || r_v->type == s_fFunction) {
                    shiro_error(0, ERR_TYPE_ERROR, "Invalid right-hand operand");
                    return NULL;
                }

                bool result = false;

                switch (l_v->type) {
                    case s_fString: {
                        shiro_string stringified = shiro_to_string(r);
                        result = strcmp(l_v->value.str, stringified) <= 0;
                        free(stringified);
                        break;
                    }
                    case s_fInteger: {
                        shiro_int intified = shiro_to_int(r);
                        result = l_v->value.i <= intified;
                        break;
                    }
                    case s_fLong: {
                        shiro_long longified = shiro_to_long(r);
                        result = l_v->value.l <= longified;
                        break;
                    }
                    case s_fUInt: {
                        shiro_uint longified = shiro_to_uint(r);
                        result = l_v->value.u <= longified;
                        break;
                    }
                    case s_fFloat: {
                        shiro_float floatified = shiro_to_float(r);
                        result = l_v->value.f <= floatified;
                        break;
                    }
                    default:
                        break;
                }

                shiro_push_value(runtime, shiro_new_int(result));

                shiro_free_value(r);
                shiro_free_value(l);
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
            case BREAK:
                do { i++; } while (binary->nodes[i]->code != END_LOOP);
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
    shiro_protect(
        shiro_execute(runtime, binary);
    );
    runtime->self = self;

    return runtime;
}
//-----------------------------------------------------------------------------
// * Executa uma função do shiro
//      f       : Função a ser chamada
//      runtime : Runtime usado para executar a função
//      n_args  : Número de argumentos para a função
//-----------------------------------------------------------------------------
SHIRO_API shiro_runtime* shiro_call_function(
    const shiro_function* f,
    shiro_runtime* runtime,
    shiro_uint n_args
) {
    if (n_args != f->n_args) {
        shiro_error(0, ERR_ARGUMENT_ERROR, "Wrong number of arguments: expected %d, got %d", f->n_args, n_args);
        return NULL;
    }

    if (f->type == s_fnShiroBinary) {
        shiro_value* old = shiro_clone_value(runtime->self);

        shiro_uint i;
        for (i = 0; i < n_args; i++) {
            shiro_value* arg = shiro_use_value(shiro_get_last_value(runtime));
            shiro_set_global(runtime, ARG(i), s_fValue, (union __field_value)shiro_use_value(arg));
            shiro_drop_value(runtime);
        }

        shiro_protect(
            shiro_execute(runtime, f->s_binary);
        );

        for (i = 0; i < n_args; i++) {
            shiro_set_global(runtime, ARG(i), s_fValue, (union __field_value)shiro_nil);

            shiro_string name = get_string(f->s_binary->nodes[i * 2 + 1]->args[1]);
            shiro_id id = ID(name);

            shiro_field* old_global = shiro_get_field(old, id);

            if (old_global != NULL)
                shiro_set_global(runtime, id, old_global->type, old_global->value);
            else
                shiro_set_global(runtime, id, s_fValue, (union __field_value)shiro_nil);
        }

        shiro_free_value(old);
    } else {
        shiro_value* returned = (*f->native)(runtime, n_args);

        if (returned == NULL)
            return NULL;

        int i;
        for (i = 0; i < n_args; i++)
            shiro_drop_value(runtime);
        shiro_push_value(runtime, returned);
    }

    return runtime;
}
