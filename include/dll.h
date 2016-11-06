#ifndef DLL_H_INCLUDED
#define DLL_H_INCLUDED

#if defined(_WIN32)
#   ifdef SHIRO_EXPORTS
#       define SHIRO_API __declspec(dllexport)
#   else
#       define SHIRO_API __declspec(dllimport)
#   endif
#else
#   define SHIRO_API
#endif

#endif // DLL_H_INCLUDED
