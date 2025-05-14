#pragma once

#include "device.h"

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkp::graphics {

    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo        viewportInfo{};
        VkPipelineInputAssemblyStateCreateInfo   inputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo   rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo     multisampleInfo{};
        VkPipelineColorBlendAttachmentState      colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo      colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo    depthStencilInfo{};
        std::vector<VkDynamicState>              dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo         dynamicStateInfo{};

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass     renderPass     = VK_NULL_HANDLE;
        uint32_t         subpass        = 0;
    };

    class Pipeline {
    public:
        Pipeline(
           Device& device,
           const std::string& vertFilepath,
           const std::string& fragFilepath,
           const PipelineConfigInfo& configInfo);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        void operator=(const Pipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer) const;

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    private:
        static std::vector<char> readFile(const std::string& filepath);

        void createGraphicsPipeline(
           const std::string& vertFilepath,
           const std::string& fragFilepath,
           const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) const;

        Device&        device;
        VkPipeline     graphicsPipeline = VK_NULL_HANDLE;
        VkShaderModule vertShaderModule = VK_NULL_HANDLE;
        VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    };
}  // namespace vkp::graphics
