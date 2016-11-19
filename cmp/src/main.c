//=============================================================================
// src\main.c
//-----------------------------------------------------------------------------
// Ponto de entrada do compilador do shiro
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

    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Syntax: shiroc <filename> [<output-file-name>]");
        return -1;
    }

    shiro_uint i = 0;
    shiro_string code = calloc(1, sizeof(shiro_character));

    shiro_string fname = argv[1];

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

    shiro_string outname;
    if (argc == 3) {
        outname = argv[2];
    } else {
        outname = calloc(512, sizeof(shiro_character));

        char* dot = strrchr(fname, '.');

        if (dot > 0 && dot > strrchr(fname, '/') && dot > strrchr(fname, '\\'))
            memcpy(outname, fname, dot - fname);

        strcat(outname, ".shr");
    }

    FILE* output = fopen(outname, "wb");

    if (output == NULL) {
            fprintf(stderr, "Failed to open output file");
        return 2;
    }

    shiro_write_binary(output, binary);
    fclose(output);

    shiro_free_binary(binary);
    free(code);

    return 0;
}
