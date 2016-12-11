//=============================================================================
// src\bignum.c
//-----------------------------------------------------------------------------
// Implementação das funções de números de precisão arbitrária do shiro
//=============================================================================
#include <shiro.h>

#include <gmp.h>

#include <stdlib.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

#if defined(__WIN32__)
__declspec(dllexport) BOOL APIENTRY DllMain(
    HINSTANCE hinstDLL,
    DWORD     fdwReason,
    LPVOID    lpvReserved
) {
    return TRUE;
}
#endif // defined

/*
//-----------------------------------------------------------------------------
// Função do shiro para criar um inteiro de precisão arbitrária
// Define o valor do inteiro como zero
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_native(bignum) {
    mpz_t* bigint = malloc(sizeof(mpz_t));
    mpz_init(*bigint);

    return shiro_new_uint((shiro_uint)bigint);
}
*/

//-----------------------------------------------------------------------------
// Função do shiro para converter um valor em bignum
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_native(bignum) {
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
shiro_native(set_bignum) {
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
shiro_native(free_bignum) {
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
shiro_native(bignum_add) {
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
shiro_native(bignum_sub) {
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
shiro_native(bignum_mul) {
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
shiro_native(bignum_cdiv) {
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
shiro_native(bignum_fdiv) {
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
shiro_native(bignum_tdiv) {
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
shiro_native(bignum_cmod) {
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
shiro_native(bignum_fmod) {
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
shiro_native(bignum_tmod) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigintA = (mpz_t*)get_uint(arg0);
    mpz_t* bigintB = (mpz_t*)get_uint(arg1);

    mpz_tdiv_r(*bigintA, *bigintA, *bigintB);

    return shiro_new_uint((shiro_uint)bigintA);
}
//-----------------------------------------------------------------------------
// Função do shiro para potênciação de inteiros de precisão arbitrária
//
// O primeiro argumento (base) deve ser um bignum e o segundo (expoente) deve
// ser um UInt
//
// Retorna um UInt representando o ponteiro para o bignum passado como base
//-----------------------------------------------------------------------------
shiro_native(bignum_pow) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigint  = (mpz_t*)get_uint(arg0);
    shiro_uint exp = shiro_to_uint(arg1);

    mpz_pow_ui(*bigint, *bigint, exp);

    return shiro_new_uint((shiro_uint)bigint);
}
//-----------------------------------------------------------------------------
// Função do shiro para obter a raíz de base n de um bignum
//
// O primeiro argumento (radicando) deve ser um bignum e o segundo (expoente)
// deve ser um UInt
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_native(bignum_root) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    mpz_t* bigint = (mpz_t*)get_uint(arg0);
    shiro_uint n = shiro_to_uint(arg1);

    mpz_root(*bigint, *bigint, n);

    return shiro_new_uint((shiro_uint)bigint);
}
//-----------------------------------------------------------------------------
// Função do shiro para obter a raíz quadrada de um bignum
//
// O primeiro argumento (radicando) deve ser um bignum e o segundo (expoente)
// deve ser um UInt
//
// O valor de retorno da função é um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_native(bignum_sqrt) {
    shiro_push_value(runtime, shiro_new_uint(2));
    return shiro_call_native(runtime, bignum_root, 2);
}
//-----------------------------------------------------------------------------
// Função do shiro para converter um bignum em string
//
// O valor de retorno da função é uma string representando o bignum
//-----------------------------------------------------------------------------
shiro_native(bignum_to_string) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    mpz_t* bigint = (mpz_t*)get_uint(arg0);

    shiro_string str = mpz_get_str(NULL, 10, *bigint);
    shiro_value* r = shiro_new_string(str);
    free(str);

    return r;
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {
    //
    // Inicialização/finalização de bignums
    //
    shiro_def_native(runtime, bignum, 1);
    shiro_def_native(runtime, set_bignum, 2);
    shiro_def_native(runtime, free_bignum, 1);

    //
    // Operações
    //
    // Todos os operadores alteram o valor do bignum passado para eles como parâmetro
    //
    shiro_def_native(runtime, bignum_add, 2);
    shiro_def_native(runtime, bignum_sub, 2);
    shiro_def_native(runtime, bignum_mul, 2);
    shiro_def_native(runtime, bignum_cdiv, 2);
    shiro_def_native(runtime, bignum_fdiv, 2);
    shiro_def_native(runtime, bignum_tdiv, 2);
    shiro_def_native(runtime, bignum_cmod, 2);
    shiro_def_native(runtime, bignum_fmod, 2);
    shiro_def_native(runtime, bignum_tmod, 2);
    shiro_def_native(runtime, bignum_pow, 2);
    shiro_def_native(runtime, bignum_root, 2);
    shiro_def_native(runtime, bignum_sqrt, 1);

    //
    // Conversão de bignums
    //
    shiro_def_native(runtime, bignum_to_string, 1);
}
