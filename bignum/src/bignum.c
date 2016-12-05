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
// Fun��o do shiro para criar um inteiro de precis�o arbitr�ria
// Define o valor do inteiro como zero
//
// O valor de retorno da fun��o � um inteiro representando o ponteiro para o
// bignum
//-----------------------------------------------------------------------------
shiro_value* shiro_bignum(shiro_runtime* runtime, shiro_uint n_args) {
    mpz_t* bigint = malloc(sizeof(mpz_t));
    mpz_init(*bigint);

    return shiro_new_uint((shiro_uint)bigint);
}
//-----------------------------------------------------------------------------
// Fun��o do shiro para converter um valor em bignum
//
// O valor de retorno da fun��o � um inteiro representando o ponteiro para o
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
// Fun��o do shiro para alterar o valor de um bignum
// Ambos os argumentos da fun��o devem ser bignums
//
// O valor de retorno da fun��o � um inteiro representando o ponteiro para o
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
// Fun��o do shiro para liberar um bignum da mem�ria
//-----------------------------------------------------------------------------
shiro_value* shiro_free_bignum(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    mpz_t* bigint = (mpz_t*)get_uint(arg0);

    mpz_clear(*bigint);
    free(bigint);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Fun��o do shiro para somar dois bignums
// Ambos os argumentos da fun��o devem ser bignums
//
// O valor de retorno da fun��o � um inteiro representando o ponteiro para o
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
// Fun��o do shiro para subtrair um bignum de outro
// Ambos os argumentos da fun��o devem ser bignums
//
// O valor de retorno da fun��o � um inteiro representando o ponteiro para o
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
// Fun��o do shiro para multiplicar dois bignums
// Ambos os argumentos da fun��o devem ser bignums
//
// O valor de retorno da fun��o � um inteiro representando o ponteiro para o
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
// Fun��o do shiro para converter um bignum em string
// Ambos os argumentos da fun��o devem ser bignums
//
// O valor de retorno da fun��o � um inteiro representando o ponteiro para o
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

    p = shiro_new_native(0, (shiro_c_function)&shiro_bignum);
    shiro_set_global(runtime, ID("new_bignum"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_to_bignum);
    shiro_set_global(runtime, ID("to_bignum"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_set_bignum);
    shiro_set_global(runtime, ID("set_bignum"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_add);
    shiro_set_global(runtime, ID("bignum_add"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_sub);
    shiro_set_global(runtime, ID("bignum_sub"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_bignum_mul);
    shiro_set_global(runtime, ID("bignum_mul"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_bignum_to_string);
    shiro_set_global(runtime, ID("bignum_to_string"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_free_bignum);
    shiro_set_global(runtime, ID("free_bignum"), s_fFunction, (union __field_value)p);
}
