#include "Interop.hpp"
#include "VulkanUtils.hpp"
#include "Utils.hpp"
#include <vulkan/vulkan_win32.h>
#include <array>

Interop::Interop(Context& context) :
    m_context(context),
    m_device(context.getDevice())
{
    createInteropSemaphores();
    createInteropTexture();
}

Interop::~Interop()
{
    vkDeviceWaitIdle(m_device);

    vkDestroyImage(m_device, m_sharedImage, nullptr);
    vkFreeMemory(m_device, m_sharedImageMemory, nullptr);
    vkDestroyImageView(m_device, m_sharedImageView, nullptr);
    vkDestroySemaphore(m_device, m_glCompleteSemaphore, nullptr);
    vkDestroySemaphore(m_device, m_vulkanCompleteSemaphore, nullptr);
}

void Interop::transformSharedImageForGLWrite(VkCommandBuffer cb)
{
    CHECK(!m_stateIsGLWrite);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_sharedImage;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    const VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    const VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    vkCmdPipelineBarrier(cb, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    m_stateIsGLWrite = true;
}

void Interop::transformSharedImageForVKRead(VkCommandBuffer cb)
{
    CHECK(m_stateIsGLWrite);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_sharedImage;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;
    const VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vkCmdPipelineBarrier(cb, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    m_stateIsGLWrite = false;
}

HANDLE Interop::getGLCompleteHandle() const
{
    return m_glCompleteSemaphoreHandle;
}

HANDLE Interop::getVKReadyHandle() const
{
    return m_vulkanCompleteSemaphoreHandle;
}

HANDLE Interop::getSharedImageMemoryHandle() const
{
    return m_sharedImageMemoryHandle;
}

uint64_t Interop::getSharedImageMemorySize() const
{
    return m_sharedImageMemorySize;
}

VkSemaphore Interop::getGLCompleteSemaphore() const
{
    return m_glCompleteSemaphore;
}

VkSemaphore Interop::getVKReadySemaphore() const
{
    return m_vulkanCompleteSemaphore;
}

VkImageView Interop::getSharedImageView() const
{
    return m_sharedImageView;
}

void Interop::createInteropSemaphores()
{
    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHRAddr = vkGetInstanceProcAddr(m_context.getInstance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(vkGetPhysicalDeviceExternalSemaphorePropertiesKHRAddr);
    CHECK(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR);

    const std::vector<VkExternalSemaphoreHandleTypeFlagBits> flags{
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT,
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT,
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT,
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT};

    VkPhysicalDeviceExternalSemaphoreInfo externalSemaphoreInfo{};
    externalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO;
    externalSemaphoreInfo.pNext = nullptr;

    VkExternalSemaphoreProperties externalSemaphoreProperties{};
    externalSemaphoreProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES;
    externalSemaphoreProperties.pNext = nullptr;

    bool found = false;
    VkPhysicalDevice physicalDevice = m_context.getPhysicalDevice();
    VkExternalSemaphoreHandleTypeFlagBits compatibleSemaphoreType;
    for (size_t i = 0; i < flags.size(); ++i)
    {
        externalSemaphoreInfo.handleType = flags[i];
        vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, &externalSemaphoreInfo, &externalSemaphoreProperties);
        if (externalSemaphoreProperties.compatibleHandleTypes & flags[i] && //
            externalSemaphoreProperties.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT)
        {
            compatibleSemaphoreType = flags[i];
            found = true;
            break;
        }
    }

    CHECK(found);

    VkExportSemaphoreCreateInfo exportSemaphoreCreateInfo{};
    exportSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
    exportSemaphoreCreateInfo.pNext = nullptr;
    exportSemaphoreCreateInfo.handleTypes = VkExternalSemaphoreHandleTypeFlags(compatibleSemaphoreType);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = &exportSemaphoreCreateInfo;

    VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_glCompleteSemaphore));
    VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_vulkanCompleteSemaphore));

    VkSemaphoreGetWin32HandleInfoKHR semaphoreGetHandleInfo{};
    semaphoreGetHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
    semaphoreGetHandleInfo.pNext = nullptr;
    semaphoreGetHandleInfo.semaphore = VK_NULL_HANDLE;
    semaphoreGetHandleInfo.handleType = compatibleSemaphoreType;

    auto vkGetSemaphoreWin32HandleKHRAddr = vkGetInstanceProcAddr(m_context.getInstance(), "vkGetSemaphoreWin32HandleKHR");
    auto vkGetSemaphoreWin32HandleKHR = PFN_vkGetSemaphoreWin32HandleKHR(vkGetSemaphoreWin32HandleKHRAddr);
    CHECK(vkGetSemaphoreWin32HandleKHR);

    semaphoreGetHandleInfo.semaphore = m_vulkanCompleteSemaphore;
    VK_CHECK(vkGetSemaphoreWin32HandleKHR(m_device, &semaphoreGetHandleInfo, &m_vulkanCompleteSemaphoreHandle));

    semaphoreGetHandleInfo.semaphore = m_glCompleteSemaphore;
    VK_CHECK(vkGetSemaphoreWin32HandleKHR(m_device, &semaphoreGetHandleInfo, &m_glCompleteSemaphoreHandle));
}

void Interop::createInteropTexture()
{
    { // Create Image
        VkExternalMemoryImageCreateInfo externalMemoryCreateInfo{};
        externalMemoryCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
        externalMemoryCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = &externalMemoryCreateInfo;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.extent.width = c_windowWidth;
        imageCreateInfo.extent.height = c_windowHeight;
        imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VK_CHECK(vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_sharedImage));
    }

    { // Allocate and bind memory
        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(m_device, m_sharedImage, &memRequirements);

        VkExportMemoryAllocateInfo exportAllocInfo{};
        exportAllocInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
        exportAllocInfo.pNext = nullptr;
        exportAllocInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;

        const MemoryTypeResult memoryTypeResult = findMemoryType(m_context.getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CHECK(memoryTypeResult.found);

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.pNext = &exportAllocInfo;
        memAllocInfo.allocationSize = memRequirements.size;
        memAllocInfo.memoryTypeIndex = memoryTypeResult.typeIndex;
        m_sharedImageMemorySize = memRequirements.size;

        VK_CHECK(vkAllocateMemory(m_device, &memAllocInfo, nullptr, &m_sharedImageMemory));
        VK_CHECK(vkBindImageMemory(m_device, m_sharedImage, m_sharedImageMemory, 0));
    }

    { // Get memory handle
        auto vkGetMemoryWin32HandleKHRAddr = vkGetInstanceProcAddr(m_context.getInstance(), "vkGetMemoryWin32HandleKHR");
        auto vkGetMemoryWin32HandleKHR = PFN_vkGetMemoryWin32HandleKHR(vkGetMemoryWin32HandleKHRAddr);
        CHECK(vkGetMemoryWin32HandleKHR);

        VkMemoryGetWin32HandleInfoKHR memoryFdInfo{};
        memoryFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
        memoryFdInfo.pNext = nullptr;
        memoryFdInfo.memory = m_sharedImageMemory;
        memoryFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
        VK_CHECK(vkGetMemoryWin32HandleKHR(m_device, &memoryFdInfo, &m_sharedImageMemoryHandle));
    }

    { // Create image view
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.image = m_sharedImage;
        viewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewCreateInfo.subresourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_sharedImageView);
    }

    { // Image layout transform
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_sharedImage;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;

        const VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        const VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        const SingleTimeCommand command = beginSingleTimeCommands(m_context.getGraphicsCommandPool(), m_device);

        vkCmdPipelineBarrier(command.commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        endSingleTimeCommands(m_context.getGraphicsQueue(), command, m_vulkanCompleteSemaphore);

        m_stateIsGLWrite = true;
    }
}
