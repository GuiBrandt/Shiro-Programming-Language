//=============================================================================
// src\stdio.c
//-----------------------------------------------------------------------------
// Implementação das funções de entrada e saída do shiro
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
// Lê uma linha da entrada padrão
//-----------------------------------------------------------------------------
shiro_native(gets) {
    shiro_string str = malloc(1024);
    gets(str);

    return shiro_new_string(str);
}
//-----------------------------------------------------------------------------
// Escreve um valor para a saida padrão
//-----------------------------------------------------------------------------
shiro_native(print) {
    shiro_push_arg(msg, 0)
    shiro_string str = shiro_to_string(msg);

    printf(str);
    if (msg->type != s_tString)
        free(str);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Escreve um valor para a saída de erros
//-----------------------------------------------------------------------------
shiro_native(error) {
    shiro_push_arg(msg, 0)
    shiro_string str = shiro_to_string(msg);
    fprintf(stderr, str);
    if (msg->type != s_tString)
        free(str);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Abre um arquivo
//
// O primeiro parâmetro deve ser uma string com o nome do arquivo e o segundo
// uma string com o modo no qual abrir o arquivo
//
// Retorna um UInt representando o ponteiro para o arquivo aberto
//-----------------------------------------------------------------------------
shiro_native(fopen) {
    shiro_push_arg_t(fname, string, 0);
    shiro_push_arg_t(mode, string,  1);

    return shiro_new_uint((shiro_uint)fopen(fname, mode));
}
//-----------------------------------------------------------------------------
// Escreve para um arquivo
//
// O primeiro argumento deve ser um UInt de ponteiro para o arquivo e o segundo
// um valor qualquer que será escrito
//-----------------------------------------------------------------------------
shiro_native(fwrite) {
    shiro_push_arg_t(fileptr, uint, 0);
    shiro_push_arg  (data,  1);

    FILE* file = (FILE*)fileptr;
    fwrite(
        get_string(data),
        sizeof(shiro_character),
        shiro_get_field(data, ID("length"))->value.u,
        file
    );

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Dá flush em um arquivo
//
// O parâmetro passado deve ser um UInt de ponteiro para o arquivo
//-----------------------------------------------------------------------------
shiro_native(fflush) {
    shiro_push_arg_t(fileptr, uint, 0);
    fflush((FILE*)fileptr);
    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Lê o conteúdo de um arquivo
//
// O parâmetro passado deve ser um UInt de ponteiro para o arquivo
//-----------------------------------------------------------------------------
shiro_native(fread) {
    shiro_push_arg_t(fileptr, uint, 0);

    FILE* file = (FILE*)fileptr;
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
// O parâmetro passado deve ser um UInt de ponteiro para o arquivo, após
// chamada esta função este UInt não poderá mais ser usado como um ponteiro de
// arquivo
//-----------------------------------------------------------------------------
shiro_native(fclose) {
    shiro_push_arg_t(fileptr, uint, 0);

    FILE* file = (FILE*)fileptr;
    fclose(file);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Apaga um arquivo
//
// O parâmetro deve ser uma string com o nome do arquivo
//-----------------------------------------------------------------------------
shiro_native(fremove) {
    shiro_push_arg_t(fname, string, 0);
    remove(fname);
    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Renomeia um arquivo
//
// O primeiro parâmetro deve ser uma string com o nome antigo do arquivo e o
// segundo uma string com o novo nome a ser dado para o arquivo
//-----------------------------------------------------------------------------
shiro_native(frename) {
    shiro_push_arg_t(old_fname, string, 0);
    shiro_push_arg_t(new_fname, string, 0);

    rename(old_fname, new_fname);
    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Cria um arquivo temporário
//
// O parâmetro passado deve ser uma string representando o modo pelo qual abrir
// o arquivo
//
// Retorna um UInt representando o ponteiro para o arquivo temporário criado
//-----------------------------------------------------------------------------
shiro_native(temp_file) {
    shiro_push_arg_t(mode, string, 0);

    return shiro_new_uint((shiro_uint)fopen(tmpnam(NULL), mode));
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {
    //
    // Funções de entrada/saída padrão
    //
    shiro_def_native(runtime, print, 1);
    shiro_def_native(runtime, error, 1);
    shiro_def_native(runtime, gets, 1);

    //
    // Funções de entrada/saída para arquivos
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
