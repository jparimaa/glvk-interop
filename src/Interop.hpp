#pragma once

#include "Context.hpp"
#include <windows.h>

class Interop final
{
public:
    Interop(Context& context);
    ~Interop();

private:
    void createInteropSemaphores();
    void createInteropTexture();

    Context& m_context;
    VkDevice m_device;

    VkSemaphore m_glComplete;
    VkSemaphore m_glReady;
    VkImage m_sharedImage;
    VkDeviceMemory m_sharedImageMemory;
    HANDLE m_memoryHandle;
    VkImageView m_sharedImageView;
};
