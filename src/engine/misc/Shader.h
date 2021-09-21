#pragma once

#include <GL/glew.h>
#include <string>
#include <iostream>

namespace RG3GE::Core {

#ifdef DEBUG_BUILD
    void CheckOGLErr(const char* stmt, const char* fname, int line) ;
#define GLCALL(stmt) do {\
stmt; \
RG3GE::Core::CheckOGLErr(#stmt, __FILE__, __LINE__); \
} while(0)
#else
#define GLCALL(s) s
#endif

    uint32_t CreateShader(
            const std::string& vertShaderSrc, 
            const std::string& fragmentShaderSrc
    );

}
