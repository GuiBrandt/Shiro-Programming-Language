#ifndef WIN32_H_INCLUDED
#define WIN32_H_INCLUDED

#include <windows.h>

#define realpath(fname, buffer) GetFullPathName(fname, 256, buffer, NULL)
#define setenv(var)             _putenv(var)

#endif // WIN32_H_INCLUDED
