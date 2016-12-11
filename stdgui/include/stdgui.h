#ifndef STDGUI_H_INCLUDED
#define STDGUI_H_INCLUDED

#include <shiro.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void shiro_glfw_error_callback(int, const char*);
void shiro_glfw_close_callback(GLFWwindow*);

shiro_native(create_window);
shiro_native(resize_window);
shiro_native(fullscreen);

shiro_native(background);
shiro_native(foregroud);

shiro_native(line_weight);
shiro_native(line);
shiro_native(rect);
shiro_native(point);
shiro_native(ellipse);

shiro_native(push_matrix);
shiro_native(pop_matrix);
shiro_native(rotate);
shiro_native(translate);

shiro_native(mouse_x);
shiro_native(mouse_y);

shiro_native(stdgui_load_advanced);

#endif // STDGUI_H_DEFINED
