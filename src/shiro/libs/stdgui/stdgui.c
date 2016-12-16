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
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358

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
// Cria uma janela
//-----------------------------------------------------------------------------
shiro_native(create_window) {
    shiro_push_arg(arg_w, 0);
    shiro_push_arg(arg_h, 1);

    shiro_uint  width = shiro_to_uint(arg_w),
                height = shiro_to_uint(arg_h);

    shiro_push_arg_t(title, string, 0);
    shiro_string title = get_string(arg2);

    if (!glfwInit()) {
        shiro_error(0, "GUIError", "Failed to initialize GLFW-3.2.1");
        return NULL;
    }

    glfwSetErrorCallback(shiro_glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    //glfwWindowHint(GLFW_SAMPLES, 4);
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
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
    if (g == NULL || g->type != s_fFunction) {
        shiro_error(0, "GUIError", "No display function defined");
        return NULL;
    }
    shiro_function* display_func = g->value.func;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);

    g = shiro_get_global(runtime, ID("setup_gui"));
    if (g != NULL && g->type == s_fFunction)
        if (shiro_call_function(g->value.func, runtime, 0) == NULL)
            return NULL;

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
shiro_native(resize_window) {
    shiro_push_arg_c(width, uint, 0);
    shiro_push_arg_c(height, uint, 1);

    shiro_uint  width = shiro_to_uint(arg_w),
                height = shiro_to_uint(arg_h);

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
shiro_native(fullscreen) {
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
// Limpa a tela com uma cor RGB
//-----------------------------------------------------------------------------
shiro_native(background) {
    shiro_push_arg_c(r, uint, 0);
    shiro_push_arg_c(g, uint, 1);
    shiro_push_arg_c(b, uint, 1);

    glClearColor(r / 255.0, g / 255.0, b / 255.0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Define a cor dos desenhos feitos na tela
//-----------------------------------------------------------------------------
shiro_native(foreground) {
    shiro_push_arg_c(r, uint, 0);
    shiro_push_arg_c(g, uint, 1);
    shiro_push_arg_c(b, uint, 1);

    glColor3f(r / 255.0, g / 255.0, b / 255.0);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Define a largura das linhas
//-----------------------------------------------------------------------------
shiro_native(line_weight) {
    shiro_push_arg(arg, 0);

    shiro_float weight = shiro_to_float(arg);
    glLineWidth(weight);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Desenha uma linha na tela
//-----------------------------------------------------------------------------
shiro_native(line) {
    shiro_push_arg_c(ax, uint, 0);
    shiro_push_arg_c(ay, uint, 1);
    shiro_push_arg_c(bx, uint, 2);
    shiro_push_arg_c(by, uint, 3);

    glBegin(GL_LINES);
        glVertex2i(ax, ay);
        glVertex2i(bx, by);
    glEnd();

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Desenha um retângulo na tela
//-----------------------------------------------------------------------------
shiro_native(rect) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1),
                *arg2 = shiro_get_value(runtime, 2),
                *arg3 = shiro_get_value(runtime, 3);

    shiro_uint  x = shiro_to_uint(arg0),
                y = shiro_to_uint(arg1),
                w = shiro_to_uint(arg2),
                h = shiro_to_uint(arg3);

    glBegin(GL_QUADS);
        glVertex2i(x, y);
        glVertex2i(x + w, y);
        glVertex2i(x + w, y + h);
        glVertex2i(x, y + h);
    glEnd();

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Desenha um ponto na tela
//-----------------------------------------------------------------------------
shiro_native(point) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_uint  x = shiro_to_uint(arg0),
                y = shiro_to_uint(arg1);

    glBegin(GL_POINTS);
        glVertex2i(x, y);
    glEnd();

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Desenha um elípse na tela
//-----------------------------------------------------------------------------
shiro_native(ellipse) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_uint  rx = shiro_to_uint(arg0),
                ry = shiro_to_uint(arg1);

    glBegin(GL_POLYGON);

    int i;
    for(i = 0; i < 360; i++)
	{
		float rad = i * PI / 180;
		glVertex2f(cos(rad) * rx, sin(rad) * ry);
	}

    glEnd();

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Salva o estado atual da matriz
//-----------------------------------------------------------------------------
shiro_native(push_matrix) {
    glPushMatrix();
    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Descarta o estado atual da matriz e volta a usar o anterior
//-----------------------------------------------------------------------------
shiro_native(pop_matrix) {
    glPopMatrix();
    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Aplica uma matriz de rotação
//-----------------------------------------------------------------------------
shiro_native(rotate) {
    shiro_value *arg0 = shiro_get_value(runtime, 0);

    shiro_float angle = shiro_to_float(arg0);

    glRotatef(angle, 0, 0, 1.0);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Aplica uma matriz de translação
//-----------------------------------------------------------------------------
shiro_native(translate) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_int   x = shiro_to_int(arg0),
                y = shiro_to_int(arg1);

    glTranslatef(x, y, 0);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Dá zoom na tela
//-----------------------------------------------------------------------------
shiro_native(scale) {
    shiro_value *arg0 = shiro_get_value(runtime, 0),
                *arg1 = shiro_get_value(runtime, 1);

    shiro_float x = shiro_to_float(arg0),
                y = shiro_to_float(arg1);

    glScalef(x, y, 1.0f);

    return shiro_nil;
}
//-----------------------------------------------------------------------------
// Obtém a posição X do mouse na janela
//-----------------------------------------------------------------------------
shiro_native(mouse_x) {
    double xpos, ypos;
    glfwGetCursorPos(shiro_window, &xpos, &ypos);

    return shiro_new_int(xpos);
}
//-----------------------------------------------------------------------------
// Obtém a posição Y do mouse na janela
//-----------------------------------------------------------------------------
shiro_native(mouse_y) {
    double xpos, ypos;
    glfwGetCursorPos(shiro_window, &xpos, &ypos);

    return shiro_new_int(ypos);
}
//-----------------------------------------------------------------------------
// Inicializa a biblioteca
//-----------------------------------------------------------------------------
shiro_main(shiro_runtime* runtime) {
    //
    // Função para uso das funções avançadas
    //
    shiro_def_native(runtime, stdgui_load_advanced, 0);

    //
    // Funções básicas
    //
    shiro_def_native(runtime, create_window, 3);
    shiro_def_native(runtime, resize_window, 2);
    shiro_def_native(runtime, fullscreen,    0);

    //
    // Funções de desenho
    //
    shiro_def_native(runtime, background, 3);
    shiro_def_native(runtime, foreground, 3);
    shiro_def_native(runtime, line_weight, 1);
    shiro_def_native(runtime, point, 2);
    shiro_def_native(runtime, line, 4);
    shiro_def_native(runtime, rect, 4);
    shiro_def_native(runtime, ellipse, 2);

    //
    // Funções de matriz
    //
    shiro_def_native(runtime, push_matrix, 0);
    shiro_def_native(runtime, pop_matrix, 0);
    shiro_def_native(runtime, rotate, 1);
    shiro_def_native(runtime, translate, 2);
    shiro_def_native(runtime, scale, 2);

    //
    // Posição do mouse
    //
    shiro_def_native(runtime, mouse_x, 0);
    shiro_def_native(runtime, mouse_y, 0);
}
