#pragma once

#include "Context.hpp"
#include <vulkan/vulkan.h>
#include <windows.h>

class Interop final
{
public:
    Interop(Context& context);
    ~Interop();

    void transformSharedImageForGLWrite(VkCommandBuffer cb);
    void transformSharedImageForVKRead(VkCommandBuffer cb);

    HANDLE getGLCompleteHandle() const;
    HANDLE getVKReadyHandle() const;
    HANDLE getSharedImageMemoryHandle() const;
    uint64_t getSharedImageMemorySize() const;
    VkSemaphore getGLCompleteSemaphore() const;
    VkSemaphore getVKReadySemaphore() const;
    VkImageView getSharedImageView() const;

private:
    void createInteropSemaphores();
    void createInteropTexture();

    Context& m_context;
    VkDevice m_device;

    VkSemaphore m_glCompleteSemaphore;
    VkSemaphore m_vulkanCompleteSemaphore;
    HANDLE m_glCompleteSemaphoreHandle;
    HANDLE m_vulkanCompleteSemaphoreHandle;
    VkImage m_sharedImage;
    uint64_t m_sharedImageMemorySize;
    VkDeviceMemory m_sharedImageMemory;
    HANDLE m_sharedImageMemoryHandle;
    VkImageView m_sharedImageView;
    bool m_stateIsGLWrite;
};
