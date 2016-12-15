//=============================================================================
// src\math.c
//-----------------------------------------------------------------------------
// Implementação das funções de matemática do shiro
//=============================================================================
#include <shiro.h>

#include <stdlib.h>
#include <math.h>

#include <time.h>

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

#define PI 3.141592653589793
#define E  2.718281828459045
//-----------------------------------------------------------------------------
// Gera um número aleatório de 0 a RANDOM_MAX
//-----------------------------------------------------------------------------
shiro_native(random) {
    shiro_int rnd = rand();
    return shiro_new_int(rnd);
}
//-----------------------------------------------------------------------------
// Obtém o valor absoluto de um número
//-----------------------------------------------------------------------------
shiro_native(abs) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(fabs(n));
}
//-----------------------------------------------------------------------------
// Obtém um número arredondado para baixo
//-----------------------------------------------------------------------------
shiro_native(floor) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(floor(n));
}
//-----------------------------------------------------------------------------
// Obtém um número arredondado para cima
//-----------------------------------------------------------------------------
shiro_native(ceil) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(ceil(n));
}
//-----------------------------------------------------------------------------
// Obtém um número arredondado
//-----------------------------------------------------------------------------
shiro_native(round) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(round(n));
}
//-----------------------------------------------------------------------------
// Obtém o maior de dois números
//-----------------------------------------------------------------------------
shiro_native(max) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    if (shiro_to_float(arg1) > shiro_to_float(arg0))
        return arg1;
    else
        return arg0;
}
//-----------------------------------------------------------------------------
// Obtém o menor de dois números
//-----------------------------------------------------------------------------
shiro_native(min) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    if (shiro_to_float(arg1) < shiro_to_float(arg0))
        return arg1;
    else
        return arg0;
}
//-----------------------------------------------------------------------------
// Mapeia um valor X, que vai de A a B, para os valores de C a D
//-----------------------------------------------------------------------------
shiro_native(map) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1),
                *arg2 = shiro_get_value(runtime, 2),
                *arg3 = shiro_get_value(runtime, 3),
                *arg4 = shiro_get_value(runtime, 4);

    shiro_float x = shiro_to_float(arg0),
                a = shiro_to_float(arg1),
                b = shiro_to_float(arg2),
                c = shiro_to_float(arg3),
                d = shiro_to_float(arg4),

                r = c + ((x - a) * d / (b - a));

    return shiro_new_uint(r);
}
//-----------------------------------------------------------------------------
// Retorna o seno de um ângulo
//-----------------------------------------------------------------------------
shiro_native(sin) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(sin(n * 180 / PI));
}
//-----------------------------------------------------------------------------
// Retorna o cosseno de um ângulo
//-----------------------------------------------------------------------------
shiro_native(cos) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(cos(n * 180 / PI));
}
//-----------------------------------------------------------------------------
// Retorna a tangente de um ângulo
//-----------------------------------------------------------------------------
shiro_native(tan) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(tan(n * 180 / PI));
}
//-----------------------------------------------------------------------------
// Retorna o arc-seno de um ângulo
//-----------------------------------------------------------------------------
shiro_native(asin) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(asin(n * 180 / PI));
}
//-----------------------------------------------------------------------------
// Retorna o arc-cosseno de um ângulo
//-----------------------------------------------------------------------------
shiro_native(acos) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(acos(n * 180 / PI));
}
//-----------------------------------------------------------------------------
// Retorna a arc-tangente de um ângulo
//-----------------------------------------------------------------------------
shiro_native(atan) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);

    shiro_float n = shiro_to_float(arg0);
    return shiro_new_float(atan(n * 180 / PI));
}

shiro_native(sinh) {
    return shiro_nil;
}

shiro_native(cosh) {
    return shiro_nil;
}

shiro_native(tanh) {
    return shiro_nil;
}

shiro_native(exp) {
    return shiro_nil;
}

shiro_native(pow) {
    return shiro_nil;
}

shiro_native(sqrt) {
    return shiro_nil;
}

shiro_native(log) {
    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {

    shiro_set_global(runtime, ID("PI"), s_fFloat, (union __field_value)PI);
    shiro_set_global(runtime, ID("E"),  s_fFloat, (union __field_value)E);

    srand((unsigned)time(NULL));

    shiro_def_native(runtime, random, 0);

}
