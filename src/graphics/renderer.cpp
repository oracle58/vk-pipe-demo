#include <vkp/graphics/renderer.h>
#include <vkp/logger.h>

#include <array>
#include <cassert>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkp::graphics {

    // Must match GLSL layout: vec2 + float, padded to 16 bytes.
    struct PushConstants {
        glm::vec2 resolution;
        float     time;
    };

    Renderer::Renderer() = default;

    Renderer::~Renderer() {
        shutdown();
    }

    bool Renderer::init(const renderer_conf& config) {
        width_     = config.start_width;
        height_    = config.start_height;

        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();

        imguiLayer = std::make_unique<vkp::ImGuiLayer>(
            window, device, *swapChain, swapChain->getRenderPass()
        );
        imguiLayer->OnAttach();
        return true;
    }

    bool Renderer::run() {
        while (!window.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(device.device());
        return true;
    }

    void Renderer::shutdown() const {
        imguiLayer->OnDetach();
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void Renderer::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
                                     | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset     = 0;
        pushConstantRange.size       = 16; // pad PushConstants up to 16

        VkPipelineLayoutCreateInfo info{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount         = 0;
        info.pSetLayouts            = nullptr;
        info.pushConstantRangeCount = 1;
        info.pPushConstantRanges    = &pushConstantRange;

        if (vkCreatePipelineLayout(
                device.device(), &info, nullptr, &pipelineLayout
            ) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void Renderer::recreateSwapChain() {
        auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device.device());

        if (swapChain == nullptr) {
            swapChain = std::make_unique<vkp::graphics::SwapChain>(device, extent);
        } else {
            swapChain = std::make_unique<vkp::graphics::SwapChain>(device, extent, std::move(swapChain));
            if (swapChain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        createPipeline();
    }

    void Renderer::createPipeline() {
        assert(swapChain && "Cannot create pipeline before swap chain");
        assert(pipelineLayout && "Cannot create pipeline before layout");

        vkp::graphics::PipelineConfigInfo conf{};
        vkp::graphics::Pipeline::defaultPipelineConfigInfo(conf);
        conf.renderPass    = swapChain->getRenderPass();
        conf.pipelineLayout = pipelineLayout;

        pipeline = std::make_unique<vkp::graphics::Pipeline>(
            device,
            "shaders/sb_shader.vert.spv",
            "shaders/sb_shader.frag.spv",
            conf
        );
    }

    void Renderer::createCommandBuffers() {
        commandBuffers.resize(swapChain->imageCount());
        VkCommandBufferAllocateInfo alloc{};
        alloc.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandPool        = device.getCommandPool();
        alloc.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(
                device.device(), &alloc, commandBuffers.data()
            ) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
            device.device(),
            device.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data()
        );
        commandBuffers.clear();
    }

    void Renderer::recordCommandBuffer(int imageIndex) const {
        static int frame = 0;
        frame = (frame + 1) % 100;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo rpInfo{};
        rpInfo.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass              = swapChain->getRenderPass();
        rpInfo.framebuffer         = swapChain->getFrameBuffer(imageIndex);
        rpInfo.renderArea.offset = {0, 0};
        rpInfo.renderArea.extent   = swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clears{};
        clears[0].color        = {{0.01f, 0.01f, 0.01f, 1.0f}};
        clears[1].depthStencil = {1.0f, 0};
        rpInfo.clearValueCount   = static_cast<uint32_t>(clears.size());
        rpInfo.pClearValues      = clears.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(swapChain->getSwapChainExtent().width);
        viewport.height   = static_cast<float>(swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        // Push constants: resolution & time
        PushConstants pc{};
        pc.resolution = { static_cast<float>(width_), static_cast<float>(height_) };
        pc.time       = static_cast<float>(glfwGetTime());
        vkCmdPushConstants(
            commandBuffers[imageIndex],
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstants),
            &pc
        );

        pipeline->bind(commandBuffers[imageIndex]);
        vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);

        imguiLayer->OnRender(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
        }
    }

    void Renderer::drawFrame() {
        uint32_t imageIndex;
        auto result = swapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swapchain image");
        }

        recordCommandBuffer(imageIndex);
        result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR
         || result == VK_SUBOPTIMAL_KHR
         || window.wasWindowResized()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swapchain image");
        }
    }

} // namespace vkp::graphics
