//=============================================================================
// src\stdgui.c
//-----------------------------------------------------------------------------
// Implementação das funções de interface gráfica do usuário do shiro
//=============================================================================
#include <shiro.h>
#include <stdgui.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>

#include <stdio.h>

#if defined(__WIN32__)
#include <windows.h>
#include <GL/wglew.h>
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

GLFWwindow* shiro_window = NULL;

//-----------------------------------------------------------------------------
// Cria uma janela
//-----------------------------------------------------------------------------
shiro_value* shiro_create_window(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1),
                *arg2 = shiro_get_value(runtime, 2);

    shiro_uint  width = shiro_to_uint(arg0),
                height = shiro_to_uint(arg1);

    shiro_string title = get_string(arg2);

    if (!glfwInit()) {
        shiro_error(0, "GUIError", "Failed to initialize GLFW-3.2.1");
        return NULL;
    }

    glfwSetErrorCallback(shiro_glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    shiro_window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (!shiro_window) {
        shiro_error(0, "GUIError", "Window creation failed");
        return NULL;
    }

    glfwMakeContextCurrent(shiro_window);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        shiro_error(0, "GUIError", "Failed to initialize GLEW-2.0.0: %s", glewGetErrorString(err));
        return NULL;
    }

    glfwSetWindowCloseCallback(shiro_window, shiro_glfw_close_callback);

    shiro_field* g = shiro_get_global(runtime, ID("display"));
    if (g == NULL) {
        shiro_error(0, "GUIError", "No display function defined");
        return NULL;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);

    shiro_function* display_func = g->value.func;

    while (shiro_window != NULL) {
        if (shiro_call_function(display_func, runtime, 0) == NULL)
            return NULL;

        glfwSwapBuffers(shiro_window);
        glfwPollEvents();
    }

    glfwTerminate();

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Redimensiona a janela
//-----------------------------------------------------------------------------
shiro_value* shiro_resize_window(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_uint  width = shiro_to_uint(arg0),
                height = shiro_to_uint(arg1);

    if (shiro_window == NULL) {
        shiro_error(0, "GUIError", "No window found to resize");
        return NULL;
    }
    glfwSetWindowSize(shiro_window, width, height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Põe a janela em tela cheia
//-----------------------------------------------------------------------------
shiro_value* shiro_fullscreen(shiro_runtime* runtime, shiro_uint n_args) {

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(
        shiro_window,
        monitor,
        0, 0,
        mode->width, mode->height,
        mode->refreshRate
    );

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Callback para erros no GLFW
//-----------------------------------------------------------------------------
void shiro_glfw_error_callback(int err, const char* desc) {
    shiro_error(0, "GUIError", "Error %d: %s", err, desc);
}
//-----------------------------------------------------------------------------
// Callback para fechamento da janela
//-----------------------------------------------------------------------------
void shiro_glfw_close_callback(GLFWwindow* window) {
    if (window != shiro_window)
        return;

    glfwDestroyWindow(shiro_window);
    shiro_window = NULL;
}
//-----------------------------------------------------------------------------
// Limpa a tela com uma cor RGB
//-----------------------------------------------------------------------------
shiro_value* shiro_background(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1),
                *arg2 = shiro_get_value(runtime, 2);

    GLuint r = shiro_to_uint(arg0);
    GLuint g = shiro_to_uint(arg1);
    GLuint b = shiro_to_uint(arg2);

    glClearColor(r / 255.0, g / 255.0, b / 255.0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Define a cor dos desenhos feitos na tela
//-----------------------------------------------------------------------------
shiro_value* shiro_foreground(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1),
                *arg2 = shiro_get_value(runtime, 2);

    GLuint r = shiro_to_uint(arg0);
    GLuint g = shiro_to_uint(arg1);
    GLuint b = shiro_to_uint(arg2);

    glColor3f(r / 255.0, g / 255.0, b / 255.0);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Define a largura das linhas
//-----------------------------------------------------------------------------
shiro_value* shiro_line_weight(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    shiro_float weight = shiro_to_float(arg0);

    glLineWidth(weight);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Desenha uma linha na tela
//-----------------------------------------------------------------------------
shiro_value* shiro_line(shiro_runtime* runtime, shiro_uint n_args) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1),
                *arg2 = shiro_get_value(runtime, 2),
                *arg3 = shiro_get_value(runtime, 3);

    shiro_uint  ax = shiro_to_uint(arg0),
                ay = shiro_to_uint(arg1),
                bx = shiro_to_uint(arg2),
                by = shiro_to_uint(arg3);

    glBegin(GL_LINES);
    glVertex2i(ax, ay);
    glVertex2i(bx, by);
    glEnd();

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {

    shiro_function* p;

    //
    // Função para uso das funções avançadas
    //
    p = shiro_new_native(0, (shiro_c_function)&shiro_stdgui_advanced);
    shiro_set_global(runtime, ID("stdgui_load_advanced"), s_fFunction, (union __field_value)p);

    //
    // Funções básicas
    //
    p = shiro_new_native(3, (shiro_c_function)&shiro_create_window);
    shiro_set_global(runtime, ID("create_window"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(2, (shiro_c_function)&shiro_resize_window);
    shiro_set_global(runtime, ID("resize_window"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(0, (shiro_c_function)&shiro_fullscreen);
    shiro_set_global(runtime, ID("fullscreen"), s_fFunction, (union __field_value)p);

    //
    // Funções de desenho
    //
    p = shiro_new_native(3, (shiro_c_function)&shiro_background);
    shiro_set_global(runtime, ID("background"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(3, (shiro_c_function)&shiro_foreground);
    shiro_set_global(runtime, ID("foreground"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(1, (shiro_c_function)&shiro_line_weight);
    shiro_set_global(runtime, ID("line_weight"), s_fFunction, (union __field_value)p);

    p = shiro_new_native(4, (shiro_c_function)&shiro_line);
    shiro_set_global(runtime, ID("line"), s_fFunction, (union __field_value)p);
}
