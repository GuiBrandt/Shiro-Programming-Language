//=============================================================================
// src\main.c
//-----------------------------------------------------------------------------
// Ponto de entrada do interpretador do shiro
//=============================================================================
#include <shiro.h>

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

//-----------------------------------------------------------------------------
// Ponto de entrada para teste
//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
    setlocale(LC_ALL, "en_US.UTF-8");

    argc = 2;

    if (argc != 2) {
        fprintf(stderr, "Syntax: shiro [filename]");
        return -1;
    }

    shiro_uint i = 0;
    shiro_string code = calloc(1, sizeof(shiro_character));

    shiro_string fname = "test/test.shiro";//argv[1];

    FILE* file = fopen(fname, "r");

    if (file == NULL) {
        fprintf(stderr, "File not found: '%s'", fname);
        return -2;
    }

    shiro_character c;
    while ((c = fgetc(file)) != EOF) {
        code[i++] = c;
        code = realloc(code, i + 1);
        code[i] = 0;
    }
    fclose(file);

    shiro_binary* binary = shiro_compile(code);

    if (binary == NULL) {
        fprintf(stderr, shiro_get_last_error());
        return 1;
    }

    shiro_runtime* runtime = shiro_init();
    shiro_load_stdlib(runtime);
    shiro_execute(runtime, binary);
    shiro_terminate(runtime);

    free(code);

    return 0;
}
