//=============================================================================
// src\advgui.c
//-----------------------------------------------------------------------------
// Implementação das funções avançadas de interface gráfica do usuário do shiro
//=============================================================================
#include <shiro.h>
#include <advgui.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>

#if defined(__WIN32__)
#include <windows.h>
#include <GL/wglew.h>
#endif

//-----------------------------------------------------------------------------
// Carrega as funções avançadas de GUI
//-----------------------------------------------------------------------------
shiro_native(stdgui_load_advanced) {
    return shiro_nil;
}
