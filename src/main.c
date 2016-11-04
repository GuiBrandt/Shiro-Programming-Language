#include "parser.h"
#include "eval.h"

#include <io.h>
#include <stdio.h>

#ifdef __DEBUG__
#include <windows.h>
#endif // __DEBUG__
//-----------------------------------------------------------------------------
// Ponto de entrada para teste
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {

    static const shiro_string code = "if (1) {\n\tprint('oe');\n} else {\n\tprint('tiao');\n};\n\nprint('asd');\nprint('bsd');";
    static const shiro_uint   iterations = 1;

    printf("Code:\n\n%s\n\n\n", code);

    double get_time() {
        LARGE_INTEGER t, f;
        QueryPerformanceCounter(&t);
        QueryPerformanceFrequency(&f);
        return (double)t.QuadPart/(double)f.QuadPart;
    }

    shiro_binary* bin = shiro_compile(code);

    printf("Iterations: %d\n", iterations);
    printf("Total of %d node(s) generated:\n", bin->used);
    int i;
    for (i = 0; i < bin->used; i++) {
        printf("    0x%02x", bin->nodes[i]->code);

        int j;
        for (j = 0; j < bin->nodes[i]->n_args; j++) {
            shiro_value* arg = bin->nodes[i]->args[j];

            if (arg->type == s_tString)
                printf(" \"%s\"", value_get_field(arg, ID("__value"))->value.str);
            else if (arg->type == s_tInt)
                printf(" %d", value_get_field(arg, ID("__value"))->value.i);
        }
        printf("\n");
    }
    printf("\n");

    double average = 0.0;
    for (i = 0; i < iterations; i++) {
        double t0 = get_time();
        shiro_binary* b = shiro_compile(code);
        double d = get_time() - t0;
        free_binary(b);
        average += d;
    }
    average /= iterations;
    printf("\nCompilation takes about %f milliseconds\n", average * 1000);

    int old_stdout = _dup(1);
    FILE* temp_out = fopen("shiro-output.txt", "w");
    _dup2(_fileno(temp_out), 1);

    shiro_runtime* runtime = shiro_init();
    {
        //
        //  Print do shiro
        //
        shiro_value* shiro_print(shiro_runtime* runtime, shiro_uint n_args) {
            printf(value_get_field(stack_get_value(runtime), ID("__value"))->value.str);
            return stack_get_value(runtime);
        }

        shiro_function* p = malloc(sizeof(shiro_function));
        p->type = s_fnNative;
        p->n_args = 1;
        p->native = (shiro_c_function)&shiro_print;
        set_global(runtime, ID("print"), s_fFunction, (union __field_value)p);
    }
    double t_exec = 0.0;
    {
        for (i = 0; i < iterations; i++) {
            double t0 = get_time();
            shiro_execute(runtime, bin);
            double d = get_time() - t0;
            t_exec += d;
        }
    }
    t_exec /= iterations;

    shiro_terminate(runtime);
    free_binary(bin);

    fflush(stdout);
    fclose(temp_out);

    _dup2(old_stdout, 1);
    _close(old_stdout);

    printf("\nOutput redirected to 'shiro-output.txt'\n");
    printf("Execution takes about %f milliseconds\n", t_exec * 1000);

    return 0;
}
