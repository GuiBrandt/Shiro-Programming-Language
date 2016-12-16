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
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {
    shiro_def_native(runtime, strlen, 1);
}
