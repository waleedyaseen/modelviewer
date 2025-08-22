#pragma once

#include <glad/glad.h>

#ifdef IMP_DEBUG
#    include <stdio.h>
#    define GLCALL(x)                                                                      \
        x;                                                                                 \
        do {                                                                               \
            GLenum glError = glGetError();                                                 \
            if (glError != GL_NO_ERROR) {                                                  \
                IMP_LOG_ERROR("OpenGL Error: %d at %s:%d\n", glError, __FILE__, __LINE__); \
            }                                                                              \
        } while (0)
#else
#    define GLCALL(x) x
#endif
