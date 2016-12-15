#ifndef WIN32_H_INCLUDED
#define WIN32_H_INCLUDED

#include <windows.h>

#define realpath(fname, buffer) GetFullPathName(fname, 256, buffer, NULL)
#define setenv(var, val, replace) char* v = malloc(strlen(val) + strlen(var) + 1);\
                                  sprintf(v, "%s=%s", var, val);\
                                  _putenv(v);

#endif // WIN32_H_INCLUDED
