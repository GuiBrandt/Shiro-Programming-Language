//=============================================================================
// src\vm.c
//-----------------------------------------------------------------------------
// Define as funções usadas para controle do runtime do shiro
//=============================================================================
#include "vm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Clona um campo shiro_field
//      f   : Ponteiro para o campo a ser clonado
//-----------------------------------------------------------------------------
shiro_field* shiro_clone_field(shiro_field* f) {

    if (f == NULL)
        return NULL;

    shiro_field* field = malloc(sizeof(shiro_field));
    memcpy(field, f, sizeof(shiro_id) + sizeof(enum __field_type));

    switch (f->type) {
        case s_fValue:
            field->value.val = shiro_use_value(f->value.val);
            break;
        case s_fString:
            if (f->value.str == NULL) break;
            shiro_uint len = strlen(f->value.str);
            field->value.str = calloc(len + 1, sizeof(shiro_character));
            memcpy(field->value.str, f->value.str, len);
            break;
        case s_fFunction:
            if (f->value.func == NULL) break;
            field->value.func = shiro_use_function(f->value.func);
            break;
        default:
            field->value = f->value;
            break;
    }

    return field;
}
//-----------------------------------------------------------------------------
// Libera um campo shiro_field da memória
//      f   : Ponteiro para o campo que será liberado
//-----------------------------------------------------------------------------
void shiro_free_field(shiro_field* f) {
    if (f == NULL)
        return;

    switch (f->type) {
        case s_fValue:
            shiro_free_value(f->value.val);
            break;
        case s_fString:
            free(f->value.str);
            break;
        case s_fFunction:
            shiro_free_function(f->value.func);
            break;
        default:
            break;
    }

    free(f);
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value sem campos e com o tipo Object
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_new_value() {
    shiro_value* val = malloc(sizeof(shiro_value));
    shiro_value v = {s_tObject, 0, NULL, 1};
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));
    return val;
}
//-----------------------------------------------------------------------------
// Marca um shiro_value como sendo usado
//      v   : shiro_value a ser marcado
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_use_value(shiro_value* v) {
    if (v == shiro_nil)
        return shiro_nil;

    v->being_used++;

    return v;
}
//-----------------------------------------------------------------------------
// Clona um shiro_value
//      v   : shiro_value a ser clonado
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_clone_value(const shiro_value* v) {

    if (v == shiro_nil)
        return shiro_nil;

    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, v, sizeof(shiro_value));
    val->being_used = 1;
    val->fields = malloc(v->n_fields * sizeof(shiro_field*));

    int i;
    for (i = 0; i < val->n_fields; i++)
        val->fields[i] = shiro_clone_field(v->fields[i]);

    return val;
}
//-----------------------------------------------------------------------------
// Obtém um shiro_id a partir de um nome
//      name    : Nome para o qual obter o ID
//-----------------------------------------------------------------------------
SHIRO_API shiro_id shiro_parse_id_from_name(const shiro_string name) {
    shiro_uint i = 0, id = 5381;
    shiro_character c;
    for (i = 0; (c = name[i]) != 0; i++)
        id = ((id << 5) + id) + c;
    return id;
}
//-----------------------------------------------------------------------------
// Define um campo de um valor a partir de um ponteiro de struct shiro_field
//      f   : shiro_field a ser definido
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_def_field(shiro_value* v, const shiro_field* f) {
    return shiro_set_field(v, f->id, f->type, f->value);
}
//-----------------------------------------------------------------------------
// Define um campo de um valor a partir de um id, um tipo de campo e um valor
//         : shiro_field a ser definido
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_set_field(
    shiro_value* v,
    const shiro_id id,
    enum __field_type t,
    const union __field_value fv)
{
    shiro_field* field;
    if ((field = shiro_get_field(v, id)) == NULL) {
        v->n_fields++;
        v->fields = realloc(v->fields, sizeof(shiro_field*) * v->n_fields);

        field = malloc(sizeof(shiro_field));
        shiro_field f = {id, t, fv};
        memcpy(field, &f, sizeof(shiro_field));

        v->fields[v->n_fields - 1] = field;
    } else {

        shiro_uint pos;
        for (pos = 0; pos < v->n_fields; pos++)
            if (v->fields[pos] == field)
                break;

        shiro_free_field(field);

        field = malloc(sizeof(shiro_field));
        shiro_field f = {id, t, fv};
        memcpy(field, &f, sizeof(shiro_field));
        field->type = t;
        field->value = fv;

        v->fields[pos] = field;
    }

    return v;
}
//-----------------------------------------------------------------------------
// Obtém um campo de um valor a partir do seu ID
//      v   : Valor do qual obter o campo
//      id  : Identificador do campo
//      pos : Ponteiro para um shiro_uint que recebe a posição do campo no
//            vetor de campos do shiro_value
//-----------------------------------------------------------------------------
SHIRO_API shiro_field* shiro_get_field(
    const shiro_value* v,
    const shiro_id id
) {
    shiro_uint i;
    for (i = 0; i < v->n_fields; i++)
        if (v->fields[i] != NULL && v->fields[i]->id == id)
            return v->fields[i];
    return NULL;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo String a partir de uma string em C
//      str : String C usada para inicializar a string do shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_new_string(const shiro_string str) {
    shiro_value v = {s_tString, 2, NULL, 1};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(2, sizeof(shiro_field*));

    shiro_uint len = strlen(str);

    shiro_string string = calloc(len + 1, sizeof(shiro_character));
    memcpy(string, str, len);

    shiro_set_field(val, ID_VALUE, s_fString, (union __field_value)string);
    shiro_set_field(val, ID("length"), s_fUInt, (union __field_value)len);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Fixnum a partir de um inteiro 32-bit
// em C
//      fix : Inteiro 32-bit C usado para inicializar o int do shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_new_int(const shiro_int fix) {
    shiro_value  v   = {s_tInt, 1, NULL, 1};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    shiro_set_field(val, ID_VALUE, s_fInteger, (union __field_value)fix);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Integer a partir de um inteiro 64-bit
// em C
//      big : Inteiro 64-bit C usado para inicializar o long do shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_new_long(const shiro_long big) {
    shiro_value  v   = {s_tInt, 1, NULL, 1};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    shiro_set_field(val, ID_VALUE, s_fLong, (union __field_value)big);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com  tipo Integer a partir de um inteiro 32-bits
// positivo em C
//      u   : Inteiro 32-bit positivo C usado para inicializar o uint do shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_new_uint(const shiro_uint u) {
    shiro_value  v   = {s_tInt, 1, NULL, 1};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    shiro_set_field(val, ID_VALUE, s_fUInt, (union __field_value)u);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Float a partir de um ponto flutuante C
//      f   : Ponto flutuante C usado para inicializar o float do shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_new_float(const shiro_float f) {
    shiro_value  v   = {s_tFloat, 1, NULL, 1};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    shiro_set_field(val, ID_VALUE, s_fFloat, (union __field_value)f);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Function a partir de uma estrutura de
// função do shiro
//      fn  : Estrutura shiro_function usada para criar o shiro_value do tipo
//            Function
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_new_function(shiro_function* fn) {
    shiro_value  v   = {s_tFunction, 1, NULL, 1};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    shiro_set_field(val, ID_VALUE, s_fFunction, (union __field_value)fn);

    return val;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por um shiro_value
//      v   : shiro_value que será liberado da memória
//-----------------------------------------------------------------------------
SHIRO_API void shiro_free_value(shiro_value* v) {
    if (v == shiro_nil)
        return;

    if (--v->being_used > 0) return;

    int i;
    for (i = 0; i < v->n_fields; i++)
        shiro_free_field(v->fields[i]);
    free(v->fields);
    free(v);
}
//-----------------------------------------------------------------------------
// Cria uma função do shiro escrita em C
//      n_args  : Número de argumentos para a função
//      fp      : Ponteiro para a função
//-----------------------------------------------------------------------------
SHIRO_API shiro_function* shiro_new_native(
    shiro_uint n_args,
    shiro_c_function fp
) {
    shiro_function* f = malloc(sizeof(shiro_function));
    f->type = s_fnNative;
    f->native = fp;
    f->n_args = n_args;
    f->being_used = 1;

    return f;
}
//-----------------------------------------------------------------------------
// Cria uma função do shiro escrita em shiro
//      n_args  : Número de argumentos para a função
//      bin     : Binário compilado do shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_function* shiro_new_fn(shiro_uint n_args, shiro_binary* bin) {
    shiro_function* f = malloc(sizeof(shiro_function));
    f->type = s_fnShiroBinary;
    f->s_binary = bin;
    f->n_args = n_args;
    f->being_used = 1;

    return f;
}
//-----------------------------------------------------------------------------
// Clona uma função do shiro
//      f   : shiro_function que será clonada
//-----------------------------------------------------------------------------
SHIRO_API shiro_function* shiro_use_function(shiro_function* f) {
    if (f == NULL)
        return NULL;

    f->being_used++;
    return f;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por uma shiro_function
//      f   : shiro_function que será liberada da memória
//-----------------------------------------------------------------------------
SHIRO_API void shiro_free_function(shiro_function* f) {

    if (f == NULL)
        return;

    if (--f->being_used > 0)
        return;

    if (f->type == s_fnShiroBinary)
         shiro_free_binary(f->s_binary);
    free(f);
}
//-----------------------------------------------------------------------------
// Inicializa um runtime do shiro
//-----------------------------------------------------------------------------
SHIRO_API shiro_runtime* shiro_init() {
    shiro_runtime* runtime = malloc(sizeof(shiro_runtime));
    runtime->used_stack = 0;

    runtime->allocated_stack = 32;

    runtime->stack = malloc(sizeof(shiro_value*) * runtime->allocated_stack);
    runtime->self = shiro_new_value();

    return runtime;
}
//-----------------------------------------------------------------------------
// Termina um runtime do shiro
//      runtime : Runtime a ser finalizado
//-----------------------------------------------------------------------------
SHIRO_API void shiro_terminate(shiro_runtime* runtime) {
    int i;
    for (i = 0; i < runtime->used_stack; i++)
        shiro_free_value(runtime->stack[i]);

    free(runtime->stack);
    shiro_free_value(runtime->self);
    free(runtime);
}
//-----------------------------------------------------------------------------
// Adiciona um valor ao topo da pilha
//      runtime : Runtime onde o valor será adicionado
//      value   : Valor que será adicionado
//-----------------------------------------------------------------------------
SHIRO_API shiro_runtime* shiro_push_value(
    shiro_runtime* runtime,
    shiro_value* value
) {
    if (value == NULL)
        return runtime;

    if (runtime->used_stack >= runtime->allocated_stack) {
        runtime->allocated_stack += runtime->used_stack;
        runtime->stack = realloc(
            runtime->stack,
            runtime->allocated_stack * sizeof(shiro_value*)
        );
    }

    runtime->stack[runtime->used_stack++] = value;

    return runtime;
}
//-----------------------------------------------------------------------------
// Remove o primeiro valor da pilha
//      runtime : Runtime de onde o valor será removido
//-----------------------------------------------------------------------------
SHIRO_API shiro_runtime* shiro_drop_value(shiro_runtime* runtime) {
    if (runtime->used_stack > 0) {
        shiro_free_value(runtime->stack[runtime->used_stack - 1]);
        runtime->used_stack--;
    }
    return runtime;
}
//-----------------------------------------------------------------------------
// Obtém o primeiro valor na pilha
//      runtime : Runtime onde o valor está
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_get_last_value(shiro_runtime* runtime) {
    if (runtime->used_stack == 0)
        return NULL;
    else
        return runtime->stack[runtime->used_stack - 1];
}
//-----------------------------------------------------------------------------
// Obtém o primeiro valor na pilha
//      runtime : Runtime onde o valor está
//-----------------------------------------------------------------------------
SHIRO_API shiro_value* shiro_get_value(shiro_runtime* runtime, shiro_uint n) {
    if (runtime->used_stack <= n)
        return NULL;
    else
        return runtime->stack[runtime->used_stack - n - 1];
}
//-----------------------------------------------------------------------------
// Define um valor global em um runtime
//      runtime : Runtime onde o global será definido
//      field   : Valor a ser definido
//-----------------------------------------------------------------------------
SHIRO_API shiro_runtime* shiro_def_global(
    shiro_runtime* runtime,
    shiro_field* field
) {
    return shiro_set_global(runtime, field->id, field->type, field->value);
}
//-----------------------------------------------------------------------------
// Define um valor global em um runtime
//      runtime : Runtime onde o global será definido
//      id      : shiro_id do global
//      g_type  : Tipo do global
//      g_val   : Valor do global
//-----------------------------------------------------------------------------
SHIRO_API shiro_runtime* shiro_set_global(
    shiro_runtime* runtime,
    shiro_id id,
    enum __field_type g_type,
    union __field_value g_val
) {
    shiro_set_field(runtime->self, id, g_type, g_val);
    return runtime;
};
//-----------------------------------------------------------------------------
// Obtém um valor global de um runtime
//      runtime : Runtime de onde o global será obtido
//      id      : Identificador do global
//-----------------------------------------------------------------------------
SHIRO_API shiro_field* shiro_get_global(shiro_runtime* runtime, shiro_id id) {
    return shiro_get_field(runtime->self, id);
}
//-----------------------------------------------------------------------------
// Determina se um valor é verdadeiro ou falso
//      val     : Valor a ser convertido
//-----------------------------------------------------------------------------
SHIRO_API bool shiro_to_bool(shiro_value* value) {
    if (value->n_fields == 0 ||
        (value->type == s_tInt && get_int(value) != 0) ||
        (value->type == s_tString &&
         shiro_get_field(value, ID("length"))->value.i != 0))
         return false;
    return true;
}
//-----------------------------------------------------------------------------
// Converte um valor do shiro em string
//      val     : Valor a ser convertido
//-----------------------------------------------------------------------------
SHIRO_API shiro_string shiro_to_string(shiro_value* val) {
    shiro_string r = calloc(32, sizeof(shiro_character));

    shiro_field* v = shiro_get_field(val, ID_VALUE);
    switch (val->type) {
        case s_tObject:
            sprintf(r, "<value @ 0x%x>", (int)val);
            return r;
        case s_tFloat:
            sprintf(r, "%f", v->value.f);
            return r;
        case s_tInt:
            switch (v->type) {
                case s_fLong:
                    sprintf(r, "%" PRId64, v->value.l);
                    return r;
                case s_fInteger:
                    sprintf(r, "%" PRId32, v->value.i);
                    return r;
                case s_fUInt:
                    sprintf(r, "%" PRIu32, v->value.u);
                    return r;
                default:
                    return "NaN";
            }
            break;
        case s_tFunction:
            sprintf(r, "<function @ 0x%x>", (int)val);
            return r;
        case s_tString:
            free(r);
            return v->value.str;
        default:
            break;
    }
    return "nil";
}
//-----------------------------------------------------------------------------
// Converte um valor do shiro em inteiro
//      val     : Valor a ser convertido
//-----------------------------------------------------------------------------
SHIRO_API shiro_int shiro_to_int(shiro_value* val) {
    shiro_field* v = shiro_get_field(val, ID_VALUE);
    switch (val->type) {
        case s_tFloat:
            return (shiro_int)v->value.f;
            break;
        case s_tInt:
            switch (v->type) {
                case s_fLong:
                    return (shiro_int)v->value.l;
                    break;
                case s_fInteger:
                    return (shiro_int)v->value.i;
                    break;
                case s_fUInt:
                    return (shiro_int)v->value.u;
                    break;
                default:
                    return 0;
            }
            break;
        case s_tFunction:
        case s_tObject:
        case s_tString:
            return (shiro_int)val;
        default:
            break;
    }
    return 0;
}
//-----------------------------------------------------------------------------
// Converte um valor do shiro em inteiro 64-bit
//      val     : Valor a ser convertido
//-----------------------------------------------------------------------------
SHIRO_API shiro_long shiro_to_long(shiro_value* val) {
    shiro_field* v = shiro_get_field(val, ID_VALUE);
    switch (val->type) {
        case s_tFloat:
            return (shiro_long)v->value.f;
            break;
        case s_tInt:
            switch (v->type) {
                case s_fLong:
                    return v->value.l;
                    break;
                case s_fInteger:
                    return (shiro_long)v->value.i;
                    break;
                case s_fUInt:
                    return (shiro_long)v->value.u;
                    break;
                default:
                    return 0;
            }
            break;
        case s_tFunction:
        case s_tObject:
        case s_tString:
            return (shiro_int)val;
        default:
            break;
    }
    return 0LL;
}
//-----------------------------------------------------------------------------
// Converte um valor do shiro em inteiro unsigned
//      val     : Valor a ser convertido
//-----------------------------------------------------------------------------
SHIRO_API shiro_uint shiro_to_uint(shiro_value* val) {
    shiro_field* v = shiro_get_field(val, ID_VALUE);
    switch (val->type) {
        case s_tFloat:
            return (shiro_uint)v->value.f;
            break;
        case s_tInt:
            switch (v->type) {
                case s_fLong:
                    return (shiro_uint)v->value.l;
                    break;
                case s_fInteger:
                    return (shiro_uint)v->value.i;
                    break;
                case s_fUInt:
                    return v->value.u;
                    break;
                default:
                    return 0;
            }
            break;
        case s_tFunction:
        case s_tObject:
        case s_tString:
            return (shiro_uint)val;
        default:
            break;
    }

    return 0U;
}
//-----------------------------------------------------------------------------
// Converte um valor do shiro em float
//      val     : Valor a ser convertido
//-----------------------------------------------------------------------------
SHIRO_API shiro_float shiro_to_float(shiro_value* val) {
    shiro_field* v = shiro_get_field(val, ID_VALUE);
    switch (val->type) {
        case s_tFloat:
            return v->value.f;
            break;
        case s_tInt:
            switch (v->type) {
                case s_fLong:
                    return (shiro_float)v->value.l;
                    break;
                case s_fInteger:
                    return (shiro_float)v->value.i;
                    break;
                case s_fUInt:
                    return (shiro_float)v->value.u;
                    break;
                default:
                    return 0.0;
            }
            break;
        case s_tFunction:
        case s_tObject:
        case s_tString:
            return 0.0;
        default:
            break;
    }

    return 0.0;
}
