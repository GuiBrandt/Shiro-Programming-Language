#include "vm.h"

shiro_value* new_value() {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tObject;
    val->n_fields = 0;
    val->fields = calloc(1, sizeof(union shiro_field));
    return val;
}

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

shiro_value* new_shiro_fixnum(const shiro_fixnum fix) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tInt;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0].i = fix;

    return val;
}

shiro_value* new_shiro_bignum(const shiro_bignum big) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tInt;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0].l = big;

    return val;
}

shiro_value* new_shiro_uint(const shiro_uint u) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tInt;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0].u = u;

    return val;
}

shiro_value* new_shiro_float(const shiro_float f) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tFloat;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));

    val->fields[0].f = f;

    return val;
}

shiro_value* new_shiro_function(const shiro_function fn) {
    shiro_value* val = malloc(sizeof(shiro_value));
    val->type = s_tString;
    val->n_fields = 1;
    val->fields = calloc(1, sizeof(union shiro_field));
    val->fields[0] = fn;

    return val;
}

void free_value(shiro_value* v) {

    switch (v->type) {

    }

    int i;
    for (i = 0; i < v->n_fields; i++) {
        free(v)
    }
    free(v);
}
