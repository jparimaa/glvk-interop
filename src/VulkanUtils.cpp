#include "VulkanUtils.hpp"
#include <GLFW/glfw3.h>
#include <set>
#include <string>
#include <fstream>

void printInstanceLayers()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layerProperties : availableLayers)
    {
        printf("%s\n", layerProperties.layerName);
    }
}

void printInstanceExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for (const auto& extension : extensions)
    {
        printf("%s\n", extension.extensionName);
    }
}

void printDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    for (const auto& extension : availableExtensions)
    {
        printf("%s\n", extension.extensionName);
    }
}

void printPhysicalDeviceName(VkPhysicalDeviceProperties properties)
{
    printf("Device name: %s\n", properties.deviceName);
}

std::vector<const char*> getRequiredInstanceExtensions()
{
    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; ++i)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    extensions.insert(extensions.end(), c_instanceExtensions.begin(), c_instanceExtensions.end());
    return extensions;
}

bool hasAllQueueFamilies(const QueueFamilyIndices& indices)
{
    return indices.graphicsFamily != -1 && indices.computeFamily != -1 && indices.presentFamily != -1;
}

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    QueueFamilyIndices indices;
    for (unsigned int i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices.computeFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (queueFamilies[i].queueCount > 0 && presentSupport)
        {
            indices.presentFamily = i;
        }

        if (hasAllQueueFamilies(indices))
        {
            break;
        }
    }

    return indices;
}

bool hasDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(c_deviceExtensions.begin(), c_deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapchainCapabilities getSwapchainCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    SwapchainCapabilities capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities.surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        capabilities.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, capabilities.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        capabilities.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, capabilities.presentModes.data());
    }

    return capabilities;
}

bool areSwapchainCapabilitiesAdequate(const SwapchainCapabilities& capabilities)
{
    return !capabilities.formats.empty() && !capabilities.presentModes.empty();
}

bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    const bool allQueueFamilies = hasAllQueueFamilies(getQueueFamilies(physicalDevice, surface));
    const bool deviceExtensionSupport = hasDeviceExtensionSupport(physicalDevice);
    const bool swapchainCapabilitiesAdequate = areSwapchainCapabilitiesAdequate(getSwapchainCapabilities(physicalDevice, surface));
    return allQueueFamilies && deviceExtensionSupport && swapchainCapabilitiesAdequate;
}

MemoryTypeResult findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    MemoryTypeResult result;
    result.found = false;

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            result.typeIndex = i;
            result.found = true;
        }
    }
    return result;
}

SingleTimeCommand beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    SingleTimeCommand command;
    command.commandBuffer = commandBuffer;
    command.commandPool = commandPool;
    command.device = device;
    return command;
}

void endSingleTimeCommands(VkQueue queue, SingleTimeCommand command, VkSemaphore signalSemaphore)
{
    VK_CHECK(vkEndCommandBuffer(command.commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command.commandBuffer;
    submitInfo.signalSemaphoreCount = signalSemaphore != VK_NULL_HANDLE ? 1 : 0;
    submitInfo.pSignalSemaphores = &signalSemaphore;

    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(queue));

    vkFreeCommandBuffers(command.device, command.commandPool, 1, &command.commandBuffer);
}

VkShaderModule createShaderModule(VkDevice device, const std::filesystem::path& path)
{
    printf("Creating shader module from %s\n", std::filesystem::absolute(path).string().c_str());

    std::ifstream file(path.string().c_str(), std::ios::ate | std::ios::binary);
    CHECK(file.is_open());

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

    return shaderModule;
}

StagingBuffer createStagingBuffer(VkDevice device, VkPhysicalDevice physicalDevice, const void* data, uint64_t size)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    const VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    const MemoryTypeResult memoryTypeResult = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
    CHECK(memoryTypeResult.found);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeResult.typeIndex;

    VkDeviceMemory memory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &memory));

    VK_CHECK(vkBindBufferMemory(device, buffer, memory, 0));

    void* dst;
    VK_CHECK(vkMapMemory(device, memory, 0, size, 0, &dst));
    std::memcpy(dst, data, static_cast<size_t>(size));
    vkUnmapMemory(device, memory);

    StagingBuffer stagingBuffer;
    stagingBuffer.buffer = buffer;
    stagingBuffer.memory = memory;

    return stagingBuffer;
}

void releaseStagingBuffer(VkDevice device, const StagingBuffer& buffer)
{
    if (buffer.buffer)
    {
        vkDestroyBuffer(device, buffer.buffer, nullptr);
    }
    if (buffer.memory)
    {
        vkFreeMemory(device, buffer.memory, nullptr);
    }
}
