#include "Context.hpp"
#include "Renderer.hpp"
#include "Utils.hpp"
#include "GLRenderer.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main(void)
{
    const int glfwInitialized = glfwInit();
    CHECK(glfwInitialized == GLFW_TRUE);

    Context context;
    Renderer renderer(context);
    GLRenderer glRenderer;

    bool running = true;
    while (running)
    {
        running = renderer.render() && glRenderer.render();
    }

    glfwTerminate();

    return 0;
}