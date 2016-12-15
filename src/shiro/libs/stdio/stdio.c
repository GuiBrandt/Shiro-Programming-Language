//=============================================================================
// src\stdio.c
//-----------------------------------------------------------------------------
// Implementa��o das fun��es de entrada e sa�da do shiro
//=============================================================================
#include <shiro.h>

#include <stdlib.h>
#include <stdio.h>

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
// L� uma linha da entrada padr�o
//-----------------------------------------------------------------------------
shiro_native(gets) {
    shiro_string str = malloc(1024);
    gets(str);

    return shiro_new_string(str);
}
//-----------------------------------------------------------------------------
// Escreve um valor para a saida padr�o
//-----------------------------------------------------------------------------
shiro_native(print) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string str = shiro_to_string(arg0);
    printf(str);
    if (arg0->type != s_tString) free(str);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Escreve um valor para a sa�da de erros
//-----------------------------------------------------------------------------
shiro_native(error) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string str = shiro_to_string(arg0);
    fprintf(stderr, str);
    if (arg0->type != s_tString) free(str);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Abre um arquivo
//
// O primeiro par�metro deve ser uma string com o nome do arquivo e o segundo
// uma string com o modo no qual abrir o arquivo
//
// Retorna um UInt representando o ponteiro para o arquivo aberto
//-----------------------------------------------------------------------------
shiro_native(fopen) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_string    fname = get_string(arg0),
                    mode = get_string(arg1);

    return shiro_new_uint((shiro_uint)fopen(fname, mode));
}
//-----------------------------------------------------------------------------
// Escreve para um arquivo
//
// O primeiro argumento deve ser um UInt de ponteiro para o arquivo e o segundo
// um valor qualquer que ser� escrito
//-----------------------------------------------------------------------------
shiro_native(fwrite) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    FILE* file = (FILE*)get_uint(arg0);
    fwrite(shiro_to_string(arg1), sizeof(shiro_character), shiro_get_field(arg1, ID("length"))->value.u, file);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// D� flush em um arquivo
//
// O par�metro passado deve ser um UInt de ponteiro para o arquivo
//-----------------------------------------------------------------------------
shiro_native(fflush) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    fflush((FILE*)get_uint(arg0));

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// L� o conte�do de um arquivo
//
// O par�metro passado deve ser um UInt de ponteiro para o arquivo
//-----------------------------------------------------------------------------
shiro_native(fread) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    FILE* file = (FILE*)get_uint(arg0);
    fseek(file, 0, SEEK_END);
    shiro_uint size = ftell(file);
    fseek(file, 0, SEEK_SET);

    shiro_string fcontents = calloc(size, sizeof(shiro_character));
    fread(fcontents, sizeof(shiro_character), size, file);

    shiro_value* str = shiro_new_string(fcontents);
    shiro_set_field(str, ID("length"), s_fUInt, (union __field_value)size);

    return str;
}
//-----------------------------------------------------------------------------
// Fecha uma arquivo
//
// O par�metro passado deve ser um UInt de ponteiro para o arquivo, ap�s
// chamada esta fun��o este UInt n�o poder� mais ser usado como um ponteiro de
// arquivo
//-----------------------------------------------------------------------------
shiro_native(fclose) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    FILE* file = (FILE*)get_uint(arg0);
    fclose(file);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Apaga um arquivo
//
// O par�metro deve ser uma string com o nome do arquivo
//-----------------------------------------------------------------------------
shiro_native(fremove) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    shiro_string fname = get_string(arg0);
    remove(fname);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Renomeia um arquivo
//
// O primeiro par�metro deve ser uma string com o nome antigo do arquivo e o
// segundo uma string com o novo nome a ser dado para o arquivo
//-----------------------------------------------------------------------------
shiro_native(frename) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_string old_fname = get_string(arg0),
                 new_fname = get_string(arg1);
    rename(old_fname, new_fname);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Cria um arquivo tempor�rio
//
// O par�metro passado deve ser uma string representando o modo pelo qual abrir
// o arquivo
//
// Retorna um UInt representando o ponteiro para o arquivo tempor�rio criado
//-----------------------------------------------------------------------------
shiro_native(temp_file) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);
    shiro_string mode = get_string(arg0);

    return shiro_new_uint((shiro_uint)fopen(tmpnam(NULL), mode));
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {

    //
    // Fun��es de entrada/sa�da padr�o
    //
    shiro_def_native(runtime, print, 1);
    shiro_def_native(runtime, error, 1);
    shiro_def_native(runtime, gets, 1);

    //
    // Fun��es de entrada/sa�da para arquivos
    //
    shiro_def_native(runtime, fopen, 2);
    shiro_def_native(runtime, fwrite, 2);
    shiro_def_native(runtime, fflush, 1);
    shiro_def_native(runtime, fread, 1);
    shiro_def_native(runtime, fclose, 1);
    shiro_def_native(runtime, fremove, 1);
    shiro_def_native(runtime, frename, 2);
    shiro_def_native(runtime, temp_file, 1);
}
