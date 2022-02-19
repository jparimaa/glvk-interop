#pragma once

#include <vector>
#include "VulkanUtils.hpp"

class GLFWwindow;

class Context final
{
public:
    Context();
    ~Context();

    VkPhysicalDevice getPhysicalDevice() const;
    VkDevice getDevice() const;
    const std::vector<VkImage>& getSwapchainImages() const;
    VkQueue getGraphicsQueue() const;
    VkCommandPool getGraphicsCommandPool() const;

    bool update();
    uint32_t acquireNextSwapchainImage();
    void submitCommandBuffers(const std::vector<VkCommandBuffer>& commandBuffers);

private:
    void initGLFW();
    void createInstance();
    void createDebugCallback();
    void createWindow();
    void handleKey(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/);
    void enumeratePhysicalDevice();
    void createDevice();
    void createSwapchain();
    void createCommandPools();
    void createSemaphores();
    void createFences();

    VkInstance m_instance;
    VkDebugReportCallbackEXT m_callback;
    GLFWwindow* m_window;
    bool m_shouldQuit = false;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkPhysicalDeviceProperties m_physicalDeviceProperties;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_computeQueue;
    VkQueue m_presentQueue;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_swapchainImages;
    VkCommandPool m_graphicsCommandPool;
    VkCommandPool m_computeCommandPool;
    VkSemaphore m_imageAvailable;
    VkSemaphore m_renderFinished;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_imageIndex;
};
