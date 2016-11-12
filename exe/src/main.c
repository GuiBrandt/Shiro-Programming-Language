#include <shiro.h>

#include <io.h>
#include <stdio.h>
#include <locale.h>

#if defined(__WIN32__)
#include <windows.h>
#else
#include <dlfcn.h>
#endif // __WIN32__

typedef void (*shiro_load_library_proc)(shiro_runtime*);

//-----------------------------------------------------------------------------
// Ponto de entrada para teste
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {

    setlocale(LC_ALL, "en_US.UTF-8");

    shiro_uint i = 0;
    shiro_string code = calloc(1, sizeof(shiro_character));

    FILE* test_file = fopen("test/test.shiro", "r");
    shiro_character c;
    while ((c = fgetc(test_file)) != EOF) {
        code[i++] = c;
        code = realloc(code, i + 1);
        code[i] = 0;
    }
    fclose(test_file);

    static const shiro_uint iterations = 1;

    printf("Code:\n\n%s\n\n\n", code);

    double get_time() {
        LARGE_INTEGER t, f;
        QueryPerformanceCounter(&t);
        QueryPerformanceFrequency(&f);
        return (double)t.QuadPart/(double)f.QuadPart;
    }

    shiro_binary* bin = shiro_compile(code);

    if (bin == NULL) {
        printf(shiro_get_last_error());
        return 1;
    }

    printf("Iterations: %d\n", iterations);
    printf("Total of %d node(s) generated:\n", bin->used);

    FILE* f = fopen("compile.log", "w");

    void __print_binary(const shiro_binary* b) {
        shiro_uint i;
        for (i = 0; i < b->used; i++) {
            fprintf(f, "0x%02x", b->nodes[i]->code);

            int j;
            for (j = 0; j < b->nodes[i]->n_args; j++) {
                shiro_value* arg = b->nodes[i]->args[j];

                if (arg->type == s_tString)
                    fprintf(f, " \"%s\"", get_string(arg));
                else if (arg->type == s_tInt)
                    fprintf(f, " %d", get_fixnum(arg));
                else if (arg->type == s_tFunction) {
                    fprintf(f, "\n<function>\n\n");
                    __print_binary(get_func(arg)->s_binary);
                    fprintf(f, "\n</function>");
                }
            }
            fprintf(f, "\n");
        }
    }
    __print_binary(bin);
    fclose(f);

    double average = 0.0;
    for (i = 0; i < iterations; i++) {
        double t0 = get_time();
        shiro_binary* b = shiro_compile(code);
        double d = get_time() - t0;
        shiro_free_binary(b);
        average += d;
    }
    average /= iterations;
    printf("\nCompilation takes about %f milliseconds\n", average * 1000);

    //int old_stdout = _dup(1);
    //FILE* temp_out = fopen("shiro-output.txt", "w");
    //_dup2(_fileno(temp_out), 1);

    shiro_runtime* runtime = shiro_init();
    {
        //
        //  Função usada para importação de bibliotecas
        //
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

        //
        //  Chamada do sistema pelo shiro
        //
        shiro_value* shiro_sys(shiro_runtime* runtime, shiro_uint n_args) {
            shiro_value* arg0 = shiro_get_value(runtime, 0);
            shiro_field* val  = shiro_get_field(arg0, ID_VALUE);

            if (val->type == s_fString)
                system(val->value.str);

            return shiro_nil;
        }

        //
        //  Converte um valor em string
        //
        shiro_value* shiro_to_str(shiro_runtime* runtime, shiro_uint n_args) {
            shiro_value* arg0 = shiro_get_value(runtime, 0);
            return shiro_new_string(shiro_to_string(arg0));
        }

        shiro_function* p;

        p = malloc(sizeof(shiro_function));
        p->type = s_fnNative;
        p->n_args = 1;
        p->native = (shiro_c_function)&shiro_sys;
        shiro_set_global(runtime, ID("sys"), s_fFunction, (union __field_value)p);

        p = malloc(sizeof(shiro_function));
        p->type = s_fnNative;
        p->n_args = 1;
        p->native = (shiro_c_function)&shiro_to_str;
        shiro_set_global(runtime, ID("to_str"), s_fFunction, (union __field_value)p);

        p = malloc(sizeof(shiro_function));
        p->type = s_fnNative;
        p->n_args = 1;
        p->native = (shiro_c_function)&shiro_import;
        shiro_set_global(runtime, ID("import"), s_fFunction, (union __field_value)p);
    }
    double t_exec = 0.0;
    {
        for (i = 0; i < iterations; i++) {
            double t0 = get_time();
            shiro_execute(runtime, bin);
            double d = get_time() - t0;

            shiro_string err;
            if ((err = shiro_get_last_error()) != NULL) {
                printf(err);
                return 1;
            }

            t_exec += d;
        }
    }
    t_exec /= iterations;

    shiro_terminate(runtime);
    shiro_free_binary(bin);

    //fflush(stdout);
    //fclose(temp_out);

    //_dup2(old_stdout, 1);
    //_close(old_stdout);

    //printf("\nOutput redirected to 'shiro-output.txt'\n");
    printf("\nExecution takes about %f milliseconds\n", t_exec * 1000);

    free(code);

    return 0;
}
