#pragma once

#include "Context.hpp"
#include <vector>

class VKRenderer final
{
public:
    VKRenderer(Context& context);
    ~VKRenderer();

    bool render();

private:
    void createRenderPass();
    void createDepthImage();
    void createImageViews();
    void createFramebuffers();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createSampler();
    void createDescriptorPool();
    void createDescriptorSet();
    void createUniformBuffer();
    void updateDescriptorSet();
    void createVertexAndIndexBuffer();
    void allocateCommandBuffers();

    Context& m_context;
    VkDevice m_device;

    VkRenderPass m_renderPass;
    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    std::vector<VkImageView> m_swapchainImageViews;
    VkImageView m_depthImageView;
    std::vector<VkFramebuffer> m_framebuffers;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkSampler m_sampler;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorSet;
    VkBuffer m_uniformBuffer;
    VkDeviceMemory m_uniformBufferMemory;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;
    std::vector<VkCommandBuffer> m_commandBuffers;
};
