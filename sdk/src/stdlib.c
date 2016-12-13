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
shiro_native(import) {
    shiro_string name = get_string(shiro_get_value(runtime, 0));
    shiro_load_library_proc proc;

    #if defined(__WIN32__)
        name = realloc(name, (strlen(name) + 2) * sizeof(shiro_character));
        strcat(name, ".");

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
shiro_native(require) {
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
shiro_native(include) {
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
shiro_native(system) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    system(shiro_to_string(arg0));

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Executa uma string como código shiro
//-----------------------------------------------------------------------------
shiro_native(eval) {
  shiro_value* arg0 = shiro_get_value(runtime, 0);

  shiro_string code = shiro_to_string(arg0);

  shiro_uint n = runtime->used_stack;

  shiro_binary* binary = shiro_compile(code);
  free(code);

  shiro_execute(runtime, binary);
  shiro_free_binary(binary);

  if (runtime->used_stack > n) {
    shiro_value* ret = shiro_get_last_value(runtime);
    shiro_drop_value(runtime);
    return ret;
  } else
    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Converte um valor em string
//-----------------------------------------------------------------------------
shiro_native(string) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_string(shiro_to_string(arg0));
}
//-----------------------------------------------------------------------------
// Converte um valor em inteiro
//-----------------------------------------------------------------------------
shiro_native(int) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_int(shiro_to_int(arg0));
}
//-----------------------------------------------------------------------------
// Converte um valor em long
//-----------------------------------------------------------------------------
shiro_native(long) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_long(shiro_to_long(arg0));
}
//-----------------------------------------------------------------------------
// Converte um valor em inteiro positivo
//-----------------------------------------------------------------------------
shiro_native(uint) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_uint(shiro_to_uint(arg0));
}
//-----------------------------------------------------------------------------
// Converte um valor em ponto flutuante
//-----------------------------------------------------------------------------
shiro_native(float) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    return shiro_new_float(shiro_to_float(arg0));
}
//-----------------------------------------------------------------------------
// Carrega as funções da biblioteca padrão do shiro para o runtime
//      runtime : shiro_runtime que será usado para executar código shiro
//-----------------------------------------------------------------------------
SHIRO_API void shiro_load_stdlib(shiro_runtime* runtime) {
    shiro_def_native(runtime, system, 1);
    shiro_def_native(runtime, string, 1);
    shiro_def_native(runtime, int, 1);
    shiro_def_native(runtime, long, 1);
    shiro_def_native(runtime, uint, 1);
    shiro_def_native(runtime, float, 1);
    shiro_def_native(runtime, import, 1);
    shiro_def_native(runtime, include, 1);
    shiro_def_native(runtime, require, 1);
    shiro_def_native(runtime, eval, 1);
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
