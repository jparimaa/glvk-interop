#include "GLRenderer.hpp"
#include "Utils.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLRenderer::GLRenderer()
{
    createWindow();
}

GLRenderer::~GLRenderer()
{
    glfwDestroyWindow(m_window);
}

bool GLRenderer::render()
{
    static float f = 0.0f;
    f += 0.0001f;
    if (f > 1.0f)
    {
        f = 0.0f;
    }
    glViewport(0, 0, c_windowWidth, c_windowHeight);
    glClearColor(0.2f, 0.3f, f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(m_window);
    return !glfwWindowShouldClose(m_window);
}

void GLRenderer::createWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_window = glfwCreateWindow(c_windowWidth, c_windowHeight, "GL", NULL, NULL);
    CHECK(m_window);

    glfwMakeContextCurrent(m_window);

    CHECK(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
}
