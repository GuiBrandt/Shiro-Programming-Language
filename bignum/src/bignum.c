#include <shiro.h>

#include <gmp.h>

#include <stdlib.h>
#include <stdio.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

#if defined(__WIN32__)
BOOL DllMain(
    HINSTANCE hinstDLL,
    DWORD     fdwReason,
    LPVOID    lpvReserved
) {
    return TRUE;
}
#endif // defined

//-----------------------------------------------------------------------------
// Função do shiro para criar um inteiro de precisão arbitrária
// Define o valor do inteiro como zero
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum(shiro_runtime* runtime, shiro_uint n_args) {
    mpz_t* bigint = malloc(sizeof(mpz_t));
    mpz_init(*bigint);

    return shiro_new_uint((shiro_uint)bigint);
}
//-----------------------------------------------------------------------------
// Função do shiro para converter um valor em bignum
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_to_bignum(shiro_runtime* runtime, shiro_uint n_args) {
    mpz_t* bigint = malloc(sizeof(mpz_t));

    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_field* val  = shiro_get_field(arg0, ID_VALUE);

    switch (val->type) {
        case s_fString:
            mpz_init_set_str(*bigint, val->value.str, 10);
            break;
        default:
            mpz_init_set_si(*bigint, shiro_to_long(arg0));
            break;
    }

    return shiro_new_uint((shiro_uint)bigint);
}
//-----------------------------------------------------------------------------
// Função do shiro para alterar o valor de um bignum
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_set_bignum(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_set(*bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para liberar um bignum da memória
//-----------------------------------------------------------------------------
shiro_value* shiro_free_bignum(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    mpz_t* bigint = (mpz_t*)get_uint(arg0);

    mpz_clear(*bigint);
    free(bigint);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Função do shiro para somar dois bignums
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_add(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_add(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para subtrair um bignum de outro
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_sub(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_sub(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para multiplicar dois bignums
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_mul(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_mul(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para dividir um bignum por outro arredondando para cima
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_cdiv(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_cdiv_q(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para dividir um bignum por outro arredondando para baixo
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_fdiv(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_fdiv_q(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para dividir um bignum por outro ignorando os decimais
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_tdiv(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_tdiv_q(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para obter o resto da divisão de um bignum por outro
// arredondando para cima
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_cmod(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_cdiv_r(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para obter o resto da divisão de um bignum por outro
// arredondando para baixo
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_fmod(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_fdiv_r(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para obter o resto da divisão de um bignum por outro
// ignorando os decimais
// Ambos os argumentos da função devem ser bignums
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_tmod(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_tdiv_r(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para converter um bignum em string
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum_to_string(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    mpz_t* bigint = (mpz_t*)get_uint(arg0);

    shiro_string str = mpz_get_str(NULL, 10, *bigint);

    return shiro_new_string(str);
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
void shiro_load_library(shiro_runtime* runtime) {
    shiro_function* p;

    p = shiro_new_native(1, (shiro_c_function)&shiro_to_bignum);
    shiro_set_global(runtime, ID("bignum"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_set_bignum);
    shiro_set_global(runtime, ID("bignum_set"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_add);
    shiro_set_global(runtime, ID("bignum_add"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_sub);
    shiro_set_global(runtime, ID("bignum_sub"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_mul);
    shiro_set_global(runtime, ID("bignum_mul"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_cdiv);
    shiro_set_global(runtime, ID("bignum_cdiv"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_fdiv);
    shiro_set_global(runtime, ID("bignum_fdiv"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_tdiv);
    shiro_set_global(runtime, ID("bignum_tdiv"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_cmod);
    shiro_set_global(runtime, ID("bignum_cmod"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_fmod);
    shiro_set_global(runtime, ID("bignum_fmod"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_tmod);
    shiro_set_global(runtime, ID("bignum_tmod"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_bignum_to_string);
    shiro_set_global(runtime, ID("bignum_to_string"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_free_bignum);
    shiro_set_global(runtime, ID("free_bignum"), s_fFunction, (union __field_value)p);
}
