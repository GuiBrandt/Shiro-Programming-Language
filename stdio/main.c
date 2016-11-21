#include <shiro.h>

#include <stdlib.h>
#include <stdio.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

#if defined(__WIN32__)
BOOL DllMain(
    HINSTANCE hinstDLL,
    DWORD     fdwReason,
    LPVOID    lpvReserved
) {
    return TRUE;
}
#endif // defined


//
//  gets do shiro
//
shiro_value* shiro_gets(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_string str = malloc(1024);
    gets(str);

    return shiro_new_string(str);
}

//
//  print do shiro
//
shiro_value* shiro_print(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_string str = shiro_to_string(arg0);
    printf(str);
    if (arg0->type != s_tString) free(str);

    return shiro_nil;
}

//
//  fopen do shiro
//
shiro_value* shiro_fopen(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value* arg0 = shiro_get_value(runtime, 0);
    shiro_value* arg1 = shiro_get_value(runtime, 1);

    shiro_string    fname = get_string(arg0),
                    mode = get_string(arg1);

    shiro_uint file = (shiro_uint)fopen(fname, mode);

    return shiro_new_uint(file);
}

//
//  fwrite do shiro
//
shiro_value* shiro_fwrite(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    FILE* file = (FILE*)get_uint(arg0);
    fwrite(get_string(arg1), sizeof(shiro_character), shiro_get_field(arg1, ID("length"))->value.u, file);

    return shiro_nil;
}

//
//  fred do shiro
//
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

//
//  fclose do shiro
//
shiro_value* shiro_fclose(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    FILE* file = (FILE*)get_uint(arg0);
    fclose(file);

    return shiro_nil;
}

//
// Inicializa a biblioteca
//
void shiro_load_library(shiro_runtime* runtime) {
    shiro_function* p;

    p = shiro_new_native(1, (shiro_c_function)&shiro_print);
    shiro_set_global(runtime, ID("print"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(0, (shiro_c_function)&shiro_gets);
    shiro_set_global(runtime, ID("gets"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_fopen);
    shiro_set_global(runtime, ID("fopen"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_fwrite);
    shiro_set_global(runtime, ID("fwrite"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_fread);
    shiro_set_global(runtime, ID("fread"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_fclose);
    shiro_set_global(runtime, ID("fclose"), s_fFunction, (union __field_value)p);
}
