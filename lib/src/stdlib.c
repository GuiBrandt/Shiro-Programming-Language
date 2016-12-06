//=============================================================================
// src\stdlib.c
//-----------------------------------------------------------------------------
// Implementação das funções da biblioteca padrão do shiro
//=============================================================================
#include <vm.h>
#include <eval.h>
#include <errors.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef __WIN32__
#include <windows.h>
#else
#include <dlfcn.h>
#endif // __WIN32__

#include <unistd.h>

#ifdef __WIN32__
#include <win32.h>
#endif // __WIN32__

#if defined(__WIN32__) && !defined(SHIRO_STATIC)
__declspec(dllexport) BOOL APIENTRY DllMain(
    HINSTANCE hinstDLL,
    DWORD     fdwReason,
    LPVOID    lpvReserved
) {
    return TRUE;
}
#endif // defined

typedef void (*shiro_load_library_proc)(shiro_runtime*);

//-----------------------------------------------------------------------------
// Função usada para importação de bibliotecas
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
            shiro_error(0, "ImportError", "'%s' is not a valid shiro library", name);
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
            shiro_error(0, "ImportError", "'%s' is not a valid shiro library", name);
            return NULL;
        }
    #endif

    proc(runtime);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Função usada para importação de arquivos compilados do shiro
//-----------------------------------------------------------------------------
shiro_value* shiro_require(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string fname = calloc(512, sizeof(shiro_character));
    sprintf(fname, get_string(arg0));

    FILE* file = fopen(fname, "rb");

    if (file == NULL && (strrchr(fname, '.') <= strrchr(fname, '/') ||
        strrchr(fname, '.') <= strrchr(fname, '\\'))) {
        strcat(fname, ".iro");
        file = fopen(fname, "rb");
    }

    if (file == NULL) {
        shiro_error(0, ERR_IO_ERROR, "No such file or directory '%s'", fname);
        return NULL;
    }

    shiro_binary* binary = shiro_read_binary(file);

    shiro_execute(runtime, binary);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Função usada para importação de arquivos .shiro
//-----------------------------------------------------------------------------
shiro_value* shiro_include(shiro_runtime* runtime, shiro_uint n_args) {
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
// Converte um valor em string
//-----------------------------------------------------------------------------
shiro_value* shiro_cast_string(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_string(shiro_to_string(arg0));
}
//-----------------------------------------------------------------------------
// Converte um valor em inteiro
//-----------------------------------------------------------------------------
shiro_value* shiro_cast_int(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_int(shiro_to_int(arg0));
}
//-----------------------------------------------------------------------------
// Converte um valor em long
//-----------------------------------------------------------------------------
shiro_value* shiro_cast_long(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_long(shiro_to_long(arg0));
}
//-----------------------------------------------------------------------------
// Converte um valor em inteiro positivo
//-----------------------------------------------------------------------------
shiro_value* shiro_cast_uint(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_uint(shiro_to_uint(arg0));
}
//-----------------------------------------------------------------------------
// Carrega as funções da biblioteca padrão do shiro para o runtime
//      runtime : shiro_runtime que será usado para executar código shiro
//-----------------------------------------------------------------------------
SHIRO_API void shiro_load_stdlib(shiro_runtime* runtime) {
    shiro_function* p;

    p = shiro_new_native(1, (shiro_c_function)&shiro_sys);
    shiro_set_global(runtime, ID("sys"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_cast_string);
    shiro_set_global(runtime, ID("string"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_cast_int);
    shiro_set_global(runtime, ID("int"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_cast_long);
    shiro_set_global(runtime, ID("long"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_cast_uint);
    shiro_set_global(runtime, ID("uint"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_import);
    shiro_set_global(runtime, ID("import"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_include);
    shiro_set_global(runtime, ID("include"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_require);
    shiro_set_global(runtime, ID("require"), s_fFunction, (union __field_value)p);
}
//-----------------------------------------------------------------------------
// Configura a variável de ambiente PATH para que o sistema de arquivos
// funcione corretamente
//      argv    : Variável argv recebida na main
//      fname   : Nome do arquivo que será executado
//-----------------------------------------------------------------------------
SHIRO_API void shiro_set_path(const shiro_string* argv, const shiro_string fname) {
    if (argv == NULL || fname == NULL)
        chdir("..");
    else {
        shiro_string full_path = calloc(256, sizeof(shiro_character));
        realpath(argv[0], full_path);

        char* e = strrchr(full_path, '\\');
        if (e == NULL)
            e = strrchr(full_path, '/');
        if (e != NULL)
            *(e + 1) = 0;

        char* curr_path = getenv("PATH");
        char* new_path = calloc(strlen(curr_path) + strlen(full_path) * 2 + 13, sizeof(char));
        sprintf(new_path, "PATH=%s;%s;%s/lib/", curr_path, full_path, full_path);
        setenv(new_path);

        free(full_path);
        free(new_path);

        full_path = calloc(256, sizeof(shiro_character));
        realpath(fname, full_path);

        e = strrchr(full_path, '\\');
        if (e == NULL)
            e = strrchr(full_path, '/');
        if (e != NULL)
            *(e + 1) = 0;

        chdir(full_path);
        free(full_path);
    }
}
