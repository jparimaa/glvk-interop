#include "Context.hpp"
#include "Interop.hpp"
#include "VKRenderer.hpp"
#include "GLRenderer.hpp"
#include "Utils.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main(void)
{
    const int glfwInitialized = glfwInit();
    CHECK(glfwInitialized == GLFW_TRUE);

    Context context;
    Interop interop(context);
    VKRenderer vkRenderer(context, interop);
    GLRenderer glRenderer(interop);

    bool running = true;
    while (running)
    {
        running = glRenderer.render() && vkRenderer.render();
    }

    glfwTerminate();

    return 0;
}