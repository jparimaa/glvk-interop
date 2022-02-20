#include "GLRenderer.hpp"
#include "Utils.hpp"

#include <GLFW/glfw3.h>

namespace
{
#define GL_HANDLE_TYPE GL_HANDLE_TYPE_OPAQUE_WIN32_EXT
} // namespace

GLRenderer::GLRenderer(Interop& interop) :
    m_interop(interop)
{
    createWindow();
}

GLRenderer::~GLRenderer()
{
    glFinish();
    glDeleteFramebuffers(1, &m_framebuffer);
    glDeleteTextures(1, &m_texture);
    glDeleteSemaphoresEXT(1, &m_vulkanCompleteSemaphore);
    glDeleteSemaphoresEXT(1, &m_glCompleteSemaphore);
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

    GLenum srcLayout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
    glWaitSemaphoreEXT(m_vulkanCompleteSemaphore, 0, nullptr, 1, &m_texture, &srcLayout);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glViewport(0, 0, c_windowWidth, c_windowHeight);
    glClearColor(0.2f, 0.3f, f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum dstLayout = GL_LAYOUT_SHADER_READ_ONLY_EXT;
    glSignalSemaphoreEXT(m_glCompleteSemaphore, 0, nullptr, 1, &m_texture, &dstLayout);

    glFlush();

    //glfwSwapBuffers(m_window);
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
    CHECK(glGenSemaphoresEXT);
}

void GLRenderer::initializeRenderer()
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glGenSemaphoresEXT(1, &m_vulkanCompleteSemaphore);
    glGenSemaphoresEXT(1, &m_glCompleteSemaphore);

    glCreateMemoryObjectsEXT(1, &m_memoryObject);

    glImportSemaphoreWin32HandleEXT(m_vulkanCompleteSemaphore, GL_HANDLE_TYPE, m_interop.getVKReadyHandle());
    glImportSemaphoreWin32HandleEXT(m_glCompleteSemaphore, GL_HANDLE_TYPE, m_interop.getGLCompleteHandle());
    glImportMemoryWin32HandleEXT(m_memoryObject, m_interop.getSharedImageMemorySize(), GL_HANDLE_TYPE, m_interop.getSharedImageMemoryHandle());

    // Use the imported memory as backing for the OpenGL texture.  The internalFormat, dimensions
    // and mip count should match the ones used by Vulkan to create the image and determine it's memory
    // allocation.
    glTextureStorageMem2DEXT(m_texture, 1, GL_RGBA8, c_windowWidth, c_windowHeight, m_memoryObject, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);

    glClearColor(0.7f, 0.3f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
