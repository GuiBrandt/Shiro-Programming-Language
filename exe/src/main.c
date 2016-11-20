//=============================================================================
// src\main.c
//-----------------------------------------------------------------------------
// Ponto de entrada do interpretador do shiro
//=============================================================================
#include <shiro.h>

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
//-----------------------------------------------------------------------------
// Ponto de entrada para teste
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
    setlocale(LC_ALL, "en_US.UTF-8");

    if (argc != 2) {
        fprintf(stderr, "Syntax: shiro <filename>");
        return -1;
    }

    shiro_uint i = 0;

    shiro_string fname = argv[1];

    FILE* file = fopen(fname, "rb");

    if (file == NULL) {
        fprintf(stderr, "File not found: '%s'", fname);
        return -2;
    }

    shiro_set_path(argv, fname);

    shiro_binary* binary;

    char* dot = strrchr(fname, '.');
    if (dot > 0 && dot >= strrchr(fname, '/') && dot >= strrchr(fname, '\\')) {
        int n;
        for (n = 0; dot[n]; n++) dot[n] = tolower(dot[n]);

        char c = fgetc(file);
        rewind(file);

        if (c != 1) {
            shiro_string code = calloc(1, sizeof(shiro_character));
            shiro_character c;
            while ((c = fgetc(file)) != EOF) {
                code[i++] = c;
                code = realloc(code, i + 1);
                code[i] = 0;
            }
            binary = shiro_compile(code);
            free(code);
        } else
            binary = shiro_read_binary(file);
    } else
        binary = shiro_read_binary(file);

    fclose(file);

    if (binary == NULL) {
        fprintf(stderr, shiro_get_last_error());
        return 1;
    }

    shiro_runtime* runtime = shiro_init();
    shiro_load_stdlib(runtime);
    shiro_execute(runtime, binary);
    shiro_terminate(runtime);

    shiro_string err = shiro_get_last_error();
    if (err != NULL) {
        fprintf(stderr, err);
        return 3;
    }

    return 0;
}
