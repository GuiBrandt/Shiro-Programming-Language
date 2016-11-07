#include <shiro.h>

#include <io.h>
#include <stdio.h>
#include <locale.h>

//#ifdef __DEBUG__
#include <windows.h>
//#endif // __DEBUG__
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

    printf("Iterations: %d\n", iterations);
    printf("Total of %d node(s) generated:\n", bin->used);

    void __print_binary(const shiro_binary* b) {
        for (i = 0; i < b->used; i++) {
            printf("    0x%02x", b->nodes[i]->code);

            int j;
            for (j = 0; j < b->nodes[i]->n_args; j++) {
                shiro_value* arg = b->nodes[i]->args[j];

                if (arg->type == s_tString)
                    printf(" \"%s\"", get_string(arg));
                else if (arg->type == s_tInt)
                    printf(" %d", get_fixnum(arg));
                else if (arg->type == s_tFunction) {
                    printf(" <function>");
                }
            }
            printf("\n");
        }
        printf("\n");
    }
    __print_binary(bin);

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

    int old_stdout = _dup(1);
    FILE* temp_out = fopen("shiro-output.txt", "w");
    _dup2(_fileno(temp_out), 1);

    shiro_runtime* runtime = shiro_init();
    {
        //
        //  Print do shiro
        //
        shiro_value* shiro_print(shiro_runtime* runtime, shiro_uint n_args) {
            shiro_value* arg0 = shiro_get_value(runtime);
            shiro_field* val  = shiro_get_field(arg0, ID_VALUE);

            switch (arg0->type) {
                case s_tObject:
                    printf("<value @ 0x%x>", (int)arg0);
                    break;
                case s_tFloat:
                    printf("%f", val->value.f);
                    break;
                case s_tInt:
                    switch (val->type) {
                        case s_fBignum:
                            printf("%ld", (long int)val->value.l);
                            break;
                        case s_fFixnum:
                            printf("%d", val->value.i);
                            break;
                        case s_fUInt:
                            printf("%u", val->value.u);
                            break;
                        default:
                            printf("NaN");
                            break;
                    }
                    break;
                case s_tFunction:
                    printf("<function @ 0x%x>", (int)arg0);
                    break;
                case s_tString:
                    printf(val->value.str);
                    break;
                default:
                    printf("nil");
                    break;
            }

            return shiro_nil;
        }

        shiro_function* p = malloc(sizeof(shiro_function));
        p->type = s_fnNative;
        p->n_args = 1;
        p->native = (shiro_c_function)&shiro_print;
        shiro_set_global(runtime, ID("print"), s_fFunction, (union __field_value)p);
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
    shiro_free_binary(bin);

    fflush(stdout);
    fclose(temp_out);

    _dup2(old_stdout, 1);
    _close(old_stdout);

    printf("\nOutput redirected to 'shiro-output.txt'\n");
    printf("Execution takes about %f milliseconds\n", t_exec * 1000);

    free(code);

    return 0;
}