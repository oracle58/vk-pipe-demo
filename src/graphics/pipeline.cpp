#include <vkp/graphics/pipeline.h>

#include <cassert>
#include <fstream>
#include <stdexcept>

namespace vkp::graphics {

    Pipeline::Pipeline(
        Device& device,
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo)
      : device{device}
    {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    Pipeline::~Pipeline() {
        if (vertShaderModule) vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
        if (fragShaderModule) vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
        if (graphicsPipeline) vkDestroyPipeline(device.device(), graphicsPipeline, nullptr);
    }

    std::vector<char> Pipeline::readFile(const std::string& filepath) {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filepath);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        return buffer;
    }

    void Pipeline::createGraphicsPipeline(
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo)
    {
        assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "No pipelineLayout in config");
        assert(configInfo.renderPass     != VK_NULL_HANDLE && "No renderPass in config");

        // --- load & compile shaders ---
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);
        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2]{};
        shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName  = "main";

        shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName  = "main";

        // --- vertex input (none) ---
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // --- assemble the pipeline ---
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState      = &configInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState   = &configInfo.multisampleInfo;
        pipelineInfo.pDepthStencilState  = &configInfo.depthStencilInfo;
        pipelineInfo.pColorBlendState    = &configInfo.colorBlendInfo;
        pipelineInfo.pDynamicState       = &configInfo.dynamicStateInfo;

        pipelineInfo.layout     = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass    = configInfo.subpass;

        if (vkCreateGraphicsPipelines(
              device.device(),
              VK_NULL_HANDLE,
              1,
              &pipelineInfo,
              nullptr,
              &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    void Pipeline::createShaderModule(
        const std::vector<char>& code,
        VkShaderModule* shaderModule) const
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());
        if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void Pipeline::bind(VkCommandBuffer commandBuffer) const {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
        // Input assembly
        configInfo.inputAssemblyInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // Viewport (dynamic)
        configInfo.viewportInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports    = nullptr;
        configInfo.viewportInfo.scissorCount  = 1;
        configInfo.viewportInfo.pScissors     = nullptr;

        // Rasterization
        configInfo.rasterizationInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable        = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode             = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth               = 1.0f;
        configInfo.rasterizationInfo.cullMode                = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable         = VK_FALSE;

        // Multisample
        configInfo.multisampleInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable  = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Color blend
        configInfo.colorBlendAttachment.colorWriteMask      =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable         = VK_FALSE;
        configInfo.colorBlendInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable             = VK_FALSE;
        configInfo.colorBlendInfo.attachmentCount           = 1;
        configInfo.colorBlendInfo.pAttachments              = &configInfo.colorBlendAttachment;

        // Depth/stencil
        configInfo.depthStencilInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp  = VK_COMPARE_OP_LESS;

        // Dynamic state
        configInfo.dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        configInfo.dynamicStateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.pDynamicStates    = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.flags             = 0;
    }

}  // namespace vkp::graphics
