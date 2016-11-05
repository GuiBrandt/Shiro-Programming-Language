#include "vm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Clona um campo shiro_field
//      f   : Ponteiro para o campo a ser clonado
//-----------------------------------------------------------------------------
shiro_field* clone_field(shiro_field* f) {
    shiro_field* field = malloc(sizeof(shiro_field));
    memcpy(field, f, sizeof(shiro_id) + sizeof(enum __field_type));

    switch (f->type) {
        case s_fValue:
            field->value.val = clone_value(f->value.val);
            break;
        case s_fString:
            if (f->value.str == NULL) break;
            shiro_uint len = strlen(f->value.str);
            field->value.str = calloc(len + 1, sizeof(shiro_character));
            memcpy(field->value.str, f->value.str, len);
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
void free_field(shiro_field* f) {
    if (f == NULL)
        return;

    switch (f->type) {
        case s_fValue:
            free(f->value.val);
            break;
        case s_fString:
            free(f->value.str);
            break;
        case s_fFunction:
            free_function(f->value.func);
            break;
        default:
            break;
    }

    free(f);
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value sem campos e com o tipo Object
//-----------------------------------------------------------------------------
shiro_value* new_value() {
    shiro_value* val = malloc(sizeof(shiro_value));
    shiro_value v = {s_tObject, 0, NULL};
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));
    return val;
}
//-----------------------------------------------------------------------------
// Clona um shiro_value
//      v   : shiro_value a ser clonado
//-----------------------------------------------------------------------------
shiro_value* clone_value(const shiro_value* v) {
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, v, sizeof(shiro_value));
    val->fields = calloc(v->n_fields, sizeof(shiro_field*));

    int i;
    for (i = 0; i < val->n_fields; i++)
        if (v->fields[i] != NULL)
            val->fields[i] = clone_field(v->fields[i]);

    return val;
}
//-----------------------------------------------------------------------------
// Obtém um shiro_id a partir de um nome
//      name    : Nome para o qual obter o ID
//-----------------------------------------------------------------------------
shiro_id __shiro_parse_id_from_name(const shiro_string name) {
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
shiro_value* value_set_field(shiro_value* v, const shiro_field* f) {
    return set_value_field(v, f->id, f->type, f->value);
}
//-----------------------------------------------------------------------------
// Define um campo de um valor a partir de um id, um tipo de campo e um valor
//         : shiro_field a ser definido
//-----------------------------------------------------------------------------
shiro_value* set_value_field(
    shiro_value* v,
    const shiro_id id,
    enum __field_type t,
    const union __field_value fv)
{
    shiro_field* field;
    if ((field = value_get_field(v, id)) == NULL) {
        v->n_fields++;
        v->fields = realloc(v->fields, sizeof(shiro_field*) * v->n_fields);

        field = malloc(sizeof(shiro_field));
        shiro_field f = {id, t, fv};
        memcpy(field, &f, sizeof(shiro_field));

        v->fields[v->n_fields - 1] = field;
    } else {
        field->type = t;
        field->value = fv;
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
shiro_field* value_get_field(
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
shiro_value* new_shiro_string(const shiro_string str) {
    shiro_value v = {s_tString, 2, NULL};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(2, sizeof(shiro_field*));

    shiro_uint len = strlen(str);

    shiro_string string = calloc(len + 1, sizeof(shiro_character));
    memcpy(string, str, len);

    set_value_field(val, ID("__value"), s_fString, (union __field_value)string);
    set_value_field(val, ID("length"), s_fUInt, (union __field_value)len);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Fixnum a partir de um inteiro 32-bit
// em C
//      fix : Inteiro 32-bit C usado para inicializar o fixnum do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_fixnum(const shiro_fixnum fix) {
    shiro_value  v   = {s_tInt, 1, NULL};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    set_value_field(val, ID("__value"), s_fFixnum, (union __field_value)fix);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Integer a partir de um inteiro 64-bit
// em C
//      big : Inteiro 64-bit C usado para inicializar o bignum do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_bignum(const shiro_bignum big) {
    shiro_value  v   = {s_tInt, 1, NULL};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    set_value_field(val, ID("__value"), s_fBignum, (union __field_value)big);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com  tipo Integer a partir de um inteiro 32-bits
// positivo em C
//      u   : Inteiro 32-bit positivo C usado para inicializar o uint do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_uint(const shiro_uint u) {
    shiro_value  v   = {s_tInt, 1, NULL};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    set_value_field(val, ID("__value"), s_fUInt, (union __field_value)u);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Float a partir de um ponto flutuante C
//      f   : Ponto flutuante C usado para inicializar o float do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_float(const shiro_float f) {
    shiro_value  v   = {s_tFloat, 1, NULL};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    set_value_field(val, ID("__value"), s_fFloat, (union __field_value)f);

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Function a partir de uma estrutura de
// função do shiro
//      fn  : Estrutura shiro_function usada para criar o shiro_value do tipo
//            Function
//-----------------------------------------------------------------------------
shiro_value* new_shiro_function(shiro_function* fn) {
    shiro_value  v   = {s_tFunction, 1, NULL};
    shiro_value* val = malloc(sizeof(shiro_value));
    memcpy(val, &v, sizeof(shiro_value));
    val->fields = calloc(1, sizeof(shiro_field*));

    set_value_field(val, ID("__value"), s_fFunction, (union __field_value)fn);

    return val;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por um shiro_value
//      v   : shiro_value que será liberado da memória
//-----------------------------------------------------------------------------
void free_value(shiro_value* v) {
    int i;
    for (i = 0; i < v->n_fields; i++)
        free_field(v->fields[i]);
    free(v->fields);
    free(v);
}
//-----------------------------------------------------------------------------
// Libera a memória usada por uma shiro_function
//      f   : shiro_function que será liberada da memória
//-----------------------------------------------------------------------------
void free_function(shiro_function* f) {
    if (f->type == s_fnShiroBinary)
        free_binary(f->s_binary);
    free(f);
}
//-----------------------------------------------------------------------------
// Inicializa um runtime do shiro
//-----------------------------------------------------------------------------
shiro_runtime* shiro_init() {
    shiro_runtime* runtime = malloc(sizeof(shiro_runtime));
    runtime->used_stack = 0;

    runtime->allocated_stack = 1;

    runtime->stack = malloc(sizeof(shiro_value*));
    runtime->self = new_value();

    return runtime;
}
//-----------------------------------------------------------------------------
// Termina um runtime do shiro
//      runtime : Runtime a ser finalizado
//-----------------------------------------------------------------------------
void shiro_terminate(shiro_runtime* runtime) {
    free(runtime->stack);
    free_value(runtime->self);
    free(runtime);
}
//-----------------------------------------------------------------------------
// Adiciona um valor ao topo da pilha
//      runtime : Runtime onde o valor será adicionado
//      value   : Valor que será adicionado
//-----------------------------------------------------------------------------
shiro_runtime* stack_push_value(shiro_runtime* runtime, shiro_value* value) {
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
shiro_runtime* stack_drop_value(shiro_runtime* runtime) {
    runtime->used_stack--;
    return runtime;
}
//-----------------------------------------------------------------------------
// Obtém o primeiro valor na pilha
//      runtime : Runtime onde o valor está
//-----------------------------------------------------------------------------
shiro_value* stack_get_value(shiro_runtime* runtime) {
    if (runtime->used_stack == 0)
        return NULL;
    else
        return runtime->stack[runtime->used_stack - 1];
}
//-----------------------------------------------------------------------------
// Define um valor global em um runtime
//      runtime : Runtime onde o global será definido
//      field   : Valor a ser definido
//-----------------------------------------------------------------------------
shiro_runtime* def_global(shiro_runtime* runtime, shiro_field* field) {
    return set_global(runtime, field->id, field->type, field->value);
}
//-----------------------------------------------------------------------------
// Define um valor global em um runtime
//      runtime : Runtime onde o global será definido
//      id      : shiro_id do global
//      g_type  : Tipo do global
//      g_val   : Valor do global
//-----------------------------------------------------------------------------
shiro_runtime* set_global(
    shiro_runtime* runtime,
    shiro_id id,
    enum __field_type g_type,
    union __field_value g_val
) {
    set_value_field(runtime->self, id, g_type, g_val);
    return runtime;
};
//-----------------------------------------------------------------------------
// Obtém um valor global de um runtime
//      runtime : Runtime de onde o global será obtido
//      id      : Identificador do global
//-----------------------------------------------------------------------------
shiro_field* get_global(shiro_runtime* runtime, shiro_id id) {
    return value_get_field(runtime->self, id);
}
