#ifndef STDGUI_H_INCLUDED
#define STDGUI_H_INCLUDED

#include <shiro.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

shiro_value* shiro_create_window    (shiro_runtime*, shiro_uint);
shiro_value* shiro_resize_window    (shiro_runtime*, shiro_uint);
shiro_value* shiro_fullscreen       (shiro_runtime*, shiro_uint);

void shiro_glfw_error_callback(int, const char*);
void shiro_glfw_close_callback(GLFWwindow*);

shiro_value* shiro_background       (shiro_runtime*, shiro_uint);
shiro_value* shiro_foreground       (shiro_runtime*, shiro_uint);

shiro_value* shiro_line_weight      (shiro_runtime*, shiro_uint);
shiro_value* shiro_line             (shiro_runtime*, shiro_uint);
shiro_value* shiro_rect             (shiro_runtime*, shiro_uint);
shiro_value* shiro_point            (shiro_runtime*, shiro_uint);
shiro_value* shiro_ellipse          (shiro_runtime*, shiro_uint);

shiro_value* shiro_push_matrix      (shiro_runtime*, shiro_uint);
shiro_value* shiro_pop_matrix       (shiro_runtime*, shiro_uint);
shiro_value* shiro_rotate           (shiro_runtime*, shiro_uint);
shiro_value* shiro_translate        (shiro_runtime*, shiro_uint);

shiro_value* shiro_stdgui_advanced  (shiro_runtime*, shiro_uint);

#endif // STDGUI_H_DEFINED
