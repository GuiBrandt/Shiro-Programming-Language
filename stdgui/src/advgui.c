//=============================================================================
// src\advgui.c
//-----------------------------------------------------------------------------
// Implementa��o das fun��es avan�adas de interface gr�fica do usu�rio do shiro
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
// Carrega as fun��es avan�adas de GUI
//-----------------------------------------------------------------------------
shiro_value* shiro_stdgui_advanced(shiro_runtime* runtime, shiro_uint n_args) {
    return shiro_nil;
}
