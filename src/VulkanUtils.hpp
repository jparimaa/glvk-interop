#pragma once

#include "Utils.hpp"
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <filesystem>

const std::vector<const char*> c_validationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> c_instanceExtensions = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME, //
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME, //
    VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, //
    VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, //
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME //
};

const std::vector<const char*> c_deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, //
    VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, //
    VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, //
    VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, //
    VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME //
};

const VkExtent2D c_windowExtent{c_windowWidth, c_windowHeight};
const VkSurfaceFormatKHR c_surfaceFormat{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
const VkFormat c_depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

#define VK_CHECK(f)                                                                             \
    do                                                                                          \
    {                                                                                           \
        const VkResult result = (f);                                                            \
        if (result != VK_SUCCESS)                                                               \
        {                                                                                       \
            printf("Abort. %s failed at %s:%d. Result = %d\n", #f, __FILE__, __LINE__, result); \
            abort();                                                                            \
        }                                                                                       \
    } while (false)

struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int computeFamily = -1;
    int presentFamily = -1;
};

struct SwapchainCapabilities
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct MemoryTypeResult
{
    bool found;
    uint32_t typeIndex;
};

struct SingleTimeCommand
{
    VkCommandPool commandPool;
    VkDevice device;
    VkCommandBuffer commandBuffer;
};

struct StagingBuffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

void printInstanceLayers();
void printInstanceExtensions();
void printDeviceExtensions(VkPhysicalDevice physicalDevice);
void printPhysicalDeviceName(VkPhysicalDeviceProperties properties);
std::vector<const char*> getRequiredInstanceExtensions();
bool hasAllQueueFamilies(const QueueFamilyIndices& indices);
QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
bool hasDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
SwapchainCapabilities getSwapchainCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
bool areSwapchainCapabilitiesAdequate(const SwapchainCapabilities& capabilities);
bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
MemoryTypeResult findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
SingleTimeCommand beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device);
void endSingleTimeCommands(VkQueue queue, SingleTimeCommand command, VkSemaphore signalSemaphore);
VkShaderModule createShaderModule(VkDevice device, const std::filesystem::path& path);
StagingBuffer createStagingBuffer(VkDevice device, VkPhysicalDevice physicalDevice, const void* data, uint64_t size);
void releaseStagingBuffer(VkDevice device, const StagingBuffer& buffer);