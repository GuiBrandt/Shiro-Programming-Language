//=============================================================================
// src\stdlib.c
//-----------------------------------------------------------------------------
// Implementa��o das fun��es da biblioteca padr�o do shiro
//=============================================================================
#include <vm.h>
#include <eval.h>
#include <errors.h>

#include <stdlib.h>
#include <stdio.h>

#if defined(__WIN32__)
#include <windows.h>
#else
#include <dlfcn.h>
#endif // __WIN32__

typedef void (*shiro_load_library_proc)(shiro_runtime*);
//-----------------------------------------------------------------------------
// Fun��o usada para importa��o de bibliotecas
//-----------------------------------------------------------------------------
shiro_value* shiro_import(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_string name = get_string(shiro_get_value(runtime, 0));
    shiro_load_library_proc proc;

    #if defined(__WIN32__)
        HMODULE library = LoadLibrary(name);
        if (library == NULL) {
            shiro_error(0, "ImportError", "Could not load library '%s'", name);
            return NULL;
        }
        proc = (shiro_load_library_proc)GetProcAddress(library, "shiro_load_library");
        if (proc == NULL) {
            shiro_error(0, "ImportError", "Failed to load 'shiro_load_library' from '%s'", name);
            return NULL;
        }
    #else
        void* library = dlopen(name, RTLD_NOW);
        if (!library) {
            shiro_error(0, "ImportError", "Could not load library '%s'", name);
            return NULL;
        }
        proc = (shiro_load_library_proc)dlsym(library, "shiro_load_library");
        int err = dlerror();
        if (err) {
            shiro_error(0, "ImportError", "Failed to load 'shiro_load_library' from '%s'", name);
            return NULL;
        }
    #endif

    proc(runtime);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Fun��o usada para importa��o de arquivos compilados do shiro
//-----------------------------------------------------------------------------
shiro_value* shiro_require(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string fname = calloc(512, sizeof(shiro_character));
    sprintf(fname, get_string(arg0));

    if (strrchr(fname, '.') <= strrchr(fname, '/') ||
        strrchr(fname, '.') <= strrchr(fname, '\\'))
        strcat(fname, ".shiro");

    FILE* file = fopen(fname, "rb");

    if (file == NULL) {
        shiro_error(0, "IOError", "No such file or directory '%s'", fname);
        return NULL;
    }

    shiro_binary* binary = shiro_read_binary(file);

    shiro_execute(runtime, binary);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Abre um arquivo .shiro no c�digo atual
//-----------------------------------------------------------------------------
shiro_value* shiro_load(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string fname = calloc(512, sizeof(shiro_character));
    sprintf(fname, get_string(arg0));

    if (strrchr(fname, '.') <= strrchr(fname, '/') ||
        strrchr(fname, '.') <= strrchr(fname, '\\'))
        strcat(fname, ".shiro");

    FILE* file = fopen(fname, "r");

    if (file == NULL) {
        shiro_error(0, ERR_IO_ERROR, "No such file or directory '%s'", fname);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    shiro_uint size = ftell(file);
    fseek(file, 0, SEEK_SET);

    shiro_string fcontents = calloc(size, sizeof(shiro_character));
    fread(fcontents, sizeof(shiro_character), size, file);

    shiro_protect(
        shiro_binary* bin = shiro_compile(fcontents);
    );

    shiro_execute(runtime, bin);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Chamada do sistema pelo shiro
//-----------------------------------------------------------------------------
shiro_value* shiro_sys(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_field* val  = shiro_get_field(arg0, ID_VALUE);

    if (val->type == s_fString)
        system(val->value.str);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
//  Converte um valor em string
//-----------------------------------------------------------------------------
shiro_value* shiro_to_str(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_string(shiro_to_string(arg0));
}
//-----------------------------------------------------------------------------
// Carrega as fun��es da biblioteca padr�o do shiro para o runtime
//      runtime : shiro_runtime que ser� usado para executar c�digo shiro
//-----------------------------------------------------------------------------
SHIRO_API void shiro_load_stdlib(shiro_runtime* runtime) {
    shiro_function* p;

    p = shiro_new_native(1, (shiro_c_function)&shiro_sys);
    shiro_set_global(runtime, ID("sys"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_to_str);
    shiro_set_global(runtime, ID("to_str"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_import);
    shiro_set_global(runtime, ID("import"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_load);
    shiro_set_global(runtime, ID("load"), s_fFunction, (union __field_value)p);
}
