#include "vm.h"

//-----------------------------------------------------------------------------
// Inicializa um shiro_value sem campos e com o tipo Object
//-----------------------------------------------------------------------------
shiro_value* new_value() {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tObject;
    val->n_fields = 0;
    val->fields = calloc(1, sizeof(union shiro_field));
    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo String a partir de uma string em C
//      str : String C usada para inicializar a string do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_string(const shiro_string str) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tString;
    val->n_fields = 2;
    val->fields = calloc(2, sizeof(union shiro_field));

    shiro_uint len = strlen(str);
    val->fields[0].str = malloc(len, sizeof(shiro_character));
    val->fields[1].u   = len;

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Fixnum a partir de um inteiro 32-bit
// em C
//      fix : Inteiro 32-bit C usado para inicializar o fixnum do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_fixnum(const shiro_fixnum fix) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tInt;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0].i = fix;

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Integer a partir de um inteiro 64-bit
// em C
//      big : Inteiro 64-bit C usado para inicializar o bignum do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_bignum(const shiro_bignum big) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tInt;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0].l = big;

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com  tipo Integer a partir de um inteiro 32-bits
// positivo em C
//      u   : Inteiro 32-bit positivo C usado para inicializar o uint do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_uint(const shiro_uint u) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tInt;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0].u = u;

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Float a partir de um ponto flutuante C
//      f   : Ponto flutuante C usado para inicializar o float do shiro
//-----------------------------------------------------------------------------
shiro_value* new_shiro_float(const shiro_float f) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tFloat;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));

    val->fields[0].f = f;

    return val;
}
//-----------------------------------------------------------------------------
// Inicializa um shiro_value com o tipo Function a partir de uma estrutura de
// função do shiro
//      fn  : Estrutura shiro_function usada para criar o shiro_value do tipo
//            Function
//-----------------------------------------------------------------------------
shiro_value* new_shiro_function(const shiro_function fn) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tFunction;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0].func = fn;

    return val;
}
//-----------------------------------------------------------------------------
// Libera a memória usada por um shiro_value
//      v   : shiro_value que será liberado da memória
//-----------------------------------------------------------------------------
void free_value(shiro_value* v) {

    switch (v->type) {
        // TODO
        default:
            break;
    }

    int i;
    for (i = 0; i < v->n_fields; i++) {
        free(v->fields[i]);
    }
    free(v);
}
