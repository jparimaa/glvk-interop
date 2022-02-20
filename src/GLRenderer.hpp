#pragma once

#include "Interop.hpp"
#include <glad/glad.h>

class GLFWwindow;

class GLRenderer final
{
public:
    GLRenderer(Interop& interop);
    ~GLRenderer();

    bool render();

private:
    void createWindow();
    void initializeRenderer();

    Interop& m_interop;
    GLFWwindow* m_window;
    GLuint m_vulkanCompleteSemaphore = 0;
    GLuint m_glCompleteSemaphore = 0;
    GLuint m_memoryObject = 0;
    GLuint m_texture = 0;
    GLuint m_framebuffer = 0;
};
