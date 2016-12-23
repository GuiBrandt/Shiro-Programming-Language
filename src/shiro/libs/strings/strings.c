//=============================================================================
// strings.c
//-----------------------------------------------------------------------------
// Implementação das funções de manipulação de strings do shiro
//=============================================================================
#include <shiro.h>

#include <stdlib.h>
#include <strings.h>

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

//-----------------------------------------------------------------------------
// Obtém o tamanho em bytes de uma string
//-----------------------------------------------------------------------------
shiro_native(strlen) {
    shiro_push_arg(arg0, 0);

    if (arg0->type == s_tString)
        return shiro_new_uint(shiro_get_field(arg0, ID("length"))->value.u);
    else {
        shiro_error(
            0,
            "TypeError",
            "Invalid type for argument passed to `strlen`"
        );
        return NULL;
    }
}
//-----------------------------------------------------------------------------
// Obtém uma sub-string
//-----------------------------------------------------------------------------
shiro_native(substr) {
    shiro_push_arg_t(str, string, 0);
    shiro_push_arg_c(start, uint, 1);
    shiro_push_arg_c(len, uint, 2);

    shiro_string res = malloc(len * sizeof(shiro_character) + 1);
    res[len * sizeof(shiro_character)] = 0;

    memcpy(res, str + start, len * sizeof(shiro_character));

    shiro_value* ret = shiro_new_string(res);
    free(res);

    return ret;
}
//-----------------------------------------------------------------------------
// Converte um número em caractere
//-----------------------------------------------------------------------------
shiro_native(char) {
    shiro_push_arg_t(n, uint, 0);

    shiro_string res = malloc(sizeof(shiro_character) + 1);
    res[sizeof(shiro_character)] = 0;

    res[0] = n;

    shiro_value* ret = shiro_new_string(res);
    free(res);

    return ret;
}
//-----------------------------------------------------------------------------
// Retorna o código ASCII de um caractere
//-----------------------------------------------------------------------------
shiro_native(ascii) {
    shiro_push_arg_t(str, string, 0);

    return shiro_new_uint(str[0]);
}
//-----------------------------------------------------------------------------
// Muda o caractere na posição n da string
//-----------------------------------------------------------------------------
shiro_native(strset) {
    shiro_push_arg(arg0, 0);
    shiro_push_arg_c(index, uint, 1);
    shiro_push_arg_c(chr, uint, 2);

    shiro_string str = get_string(arg0);
    str[index * sizeof(shiro_character)] = chr;

    return arg0;
}
//-----------------------------------------------------------------------------
// Clona uma string
//-----------------------------------------------------------------------------
shiro_native(strdup) {
    shiro_push_arg_t(str, string, 0);
    return shiro_new_string(str);
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {
    shiro_def_native(runtime, strlen, 1);
    shiro_def_native(runtime, substr, 3);
    shiro_def_native(runtime, strset, 3);
    shiro_def_native(runtime, strdup, 3);

    shiro_def_native(runtime, char, 1);
    shiro_def_native(runtime, ascii, 1);
}
