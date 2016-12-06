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
shiro_value* shiro_gets(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_string str = malloc(1024);
    gets(str);

    return shiro_new_string(str);
}
//-----------------------------------------------------------------------------
// Escreve um valor para a saida padrão
//-----------------------------------------------------------------------------
shiro_value* shiro_print(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string str = shiro_to_string(arg0);
    printf(str);
    if (arg0->type != s_tString) free(str);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Escreve um valor para a saída de erros
//-----------------------------------------------------------------------------
shiro_value* shiro_print_error(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string str = shiro_to_string(arg0);
    fprintf(stderr, str);
    if (arg0->type != s_tString) free(str);

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
shiro_value* shiro_fopen(shiro_runtime* runtime, shiro_uint n_args) {
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
// um valor qualquer que será escrito
//-----------------------------------------------------------------------------
shiro_value* shiro_fwrite(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    FILE* file = (FILE*)get_uint(arg0);
    fwrite(shiro_to_string(arg1), sizeof(shiro_character), shiro_get_field(arg1, ID("length"))->value.u, file);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Dá flush em um arquivo
//
// O parâmetro passado deve ser um UInt de ponteiro para o arquivo
//-----------------------------------------------------------------------------
shiro_value* shiro_fflush(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    fflush((FILE*)get_uint(arg0));

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Lê o conteúdo de um arquivo
//
// O parâmetro passado deve ser um UInt de ponteiro para o arquivo
//-----------------------------------------------------------------------------
shiro_value* shiro_fread(shiro_runtime* runtime, shiro_uint n_args) {
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
// O parâmetro passado deve ser um UInt de ponteiro para o arquivo, após
// chamada esta função este UInt não poderá mais ser usado como um ponteiro de
// arquivo
//-----------------------------------------------------------------------------
shiro_value* shiro_fclose(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    FILE* file = (FILE*)get_uint(arg0);
    fclose(file);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Apaga um arquivo
//
// O parâmetro deve ser uma string com o nome do arquivo
//-----------------------------------------------------------------------------
shiro_value* shiro_fremove(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    shiro_string fname = get_string(arg0);
    remove(fname);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Renomeia um arquivo
//
// O primeiro parâmetro deve ser uma string com o nome antigo do arquivo e o
// segundo uma string com o novo nome a ser dado para o arquivo
//-----------------------------------------------------------------------------
shiro_value* shiro_frename(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_string old_fname = get_string(arg0),
                 new_fname = get_string(arg1);
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
shiro_value* shiro_tmpfile(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);
    shiro_string mode = get_string(arg0);

    return shiro_new_uint((shiro_uint)fopen(tmpnam(NULL), mode));
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
SHIRO_LIB_SETUP(shiro_runtime* runtime) {
    shiro_function* p;

    p = shiro_new_native(1, (shiro_c_function)&shiro_print);
    shiro_set_global(runtime, ID("print"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_print_error);
    shiro_set_global(runtime, ID("error"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(0, (shiro_c_function)&shiro_gets);
    shiro_set_global(runtime, ID("gets"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_fopen);
    shiro_set_global(runtime, ID("fopen"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_fwrite);
    shiro_set_global(runtime, ID("fwrite"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_fwrite);
    shiro_set_global(runtime, ID("fflush"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_fread);
    shiro_set_global(runtime, ID("fread"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_fclose);
    shiro_set_global(runtime, ID("fclose"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_fremove);
    shiro_set_global(runtime, ID("fdelete"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_frename);
    shiro_set_global(runtime, ID("frename"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_tmpfile);
    shiro_set_global(runtime, ID("temp_file"), s_fFunction, (union __field_value)p);
}
