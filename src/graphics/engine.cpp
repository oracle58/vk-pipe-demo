#include <vkp/graphics/engine.h>
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

    Engine::Engine() = default;

    Engine::~Engine() {
        LOG_DEBUG("Engine destructor called");
        shutdown();
    }

    bool Engine::init(const engine_conf& config) {
        LOG_INFO("Initializing Engine with width={}, height={}, name={}", config.start_width, config.start_height, config.name);
        width_     = config.start_width;
        height_    = config.start_height;

        try {
            // Construct window and device with config values
            LOG_DEBUG("Creating Window");
            window = std::make_unique<Window>(
                config.start_width,
                config.start_height,
                config.name,
                config.start_pos_x,
                config.start_pos_y
            );
            LOG_DEBUG("Window created");

            if (!window) {
                LOG_FATAL("Window creation failed!");
                return false;
            }

            LOG_DEBUG("Creating Device");
            device = std::make_unique<vkp::graphics::Device>(*window);
            LOG_DEBUG("Device created");

            createPipelineLayout();
            recreateSwapChain();
            createCommandBuffers();

            LOG_DEBUG("Creating ImGuiLayer");
            imguiLayer = std::make_unique<vkp::ImGuiLayer>(
                *window, *device, *swapChain, swapChain->getRenderPass()
            );
            imguiLayer->OnAttach();
            LOG_INFO("Engine initialized successfully");
            return true;
        } catch (const std::exception& e) {
            LOG_FATAL("Engine initialization failed: {}", e.what());
            return false;
        }
    }

    bool Engine::run() {
        LOG_INFO("Engine run loop started");
        try {
            while (!window->shouldClose()) {
                glfwPollEvents();
                drawFrame();
            }
            LOG_DEBUG("Waiting for device to be idle");
            vkDeviceWaitIdle(device->device());
            LOG_INFO("Engine run loop exited normally");
            return true;
        } catch (const std::exception& e) {
            LOG_FATAL("Exception in run loop: {}", e.what());
            return false;
        }
    }

    void Engine::shutdown() const {
        LOG_INFO("Shutting down Engine");
        if (imguiLayer) {
            imguiLayer->OnDetach();
            LOG_DEBUG("ImGuiLayer detached");
        }
        if (device && pipelineLayout) {
            vkDestroyPipelineLayout(device->device(), pipelineLayout, nullptr);
            LOG_DEBUG("Pipeline layout destroyed");
        }
        LOG_INFO("Engine shutdown complete");
    }

    void Engine::createPipelineLayout() {
        LOG_DEBUG("Creating pipeline layout");
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

        VkResult res = vkCreatePipelineLayout(
                device->device(), &info, nullptr, &pipelineLayout
            );
        if (res != VK_SUCCESS) {
            LOG_FATAL("Failed to create pipeline layout: VkResult={}", static_cast<int>(res));
            throw std::runtime_error("failed to create pipeline layout");
        }
        LOG_DEBUG("Pipeline layout created");
    }

    void Engine::recreateSwapChain() {
        LOG_INFO("Recreating swap chain");
        auto extent = window->getExtent();
        while (extent.width == 0 || extent.height == 0) {
            LOG_WARN("Window minimized or extent zero ({}x{}), waiting...", extent.width, extent.height);
            extent = window->getExtent();
            glfwWaitEvents();
        }
        LOG_DEBUG("Waiting for device to be idle before swapchain recreation");
        vkDeviceWaitIdle(device->device());

        if (swapChain == nullptr) {
            LOG_DEBUG("Creating new SwapChain");
            swapChain = std::make_unique<graphics::SwapChain>(*device, extent);
        } else {
            LOG_DEBUG("Recreating SwapChain with old swapchain");
            swapChain = std::make_unique<graphics::SwapChain>(*device, extent, std::move(swapChain));
            if (swapChain->imageCount() != commandBuffers.size()) {
                LOG_DEBUG("Image count changed ({} -> {}), recreating command buffers", commandBuffers.size(), swapChain->imageCount());
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        createPipeline();
        LOG_INFO("Swap chain recreated");
    }

    void Engine::createPipeline() {
        LOG_DEBUG("Creating graphics pipeline");
        assert(swapChain && "Cannot create pipeline before swap chain");
        assert(pipelineLayout && "Cannot create pipeline before layout");

        graphics::PipelineConfigInfo conf{};
        graphics::Pipeline::defaultPipelineConfigInfo(conf);
        conf.renderPass    = swapChain->getRenderPass();
        conf.pipelineLayout = pipelineLayout;

        try {
            pipeline = std::make_unique<graphics::Pipeline>(
                *device,
                "shaders/sb_shader.vert.spv",
                "shaders/sb_shader.frag.spv",
                conf
            );
            LOG_INFO("Graphics pipeline created");
        } catch (const std::exception& e) {
            LOG_FATAL("Failed to create graphics pipeline: {}", e.what());
            throw;
        }
    }

    void Engine::createCommandBuffers() {
        LOG_DEBUG("Allocating {} command buffers", swapChain->imageCount());
        commandBuffers.resize(swapChain->imageCount());
        VkCommandBufferAllocateInfo alloc{};
        alloc.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandPool        = device->getCommandPool();
        alloc.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        VkResult res = vkAllocateCommandBuffers(
                device->device(), &alloc, commandBuffers.data()
            );
        if (res != VK_SUCCESS) {
            LOG_FATAL("Failed to allocate command buffers: VkResult={}", static_cast<int>(res));
            throw std::runtime_error("failed to allocate command buffers");
        }
        LOG_DEBUG("Command buffers allocated");
    }

    void Engine::freeCommandBuffers() {
        LOG_DEBUG("Freeing {} command buffers", commandBuffers.size());
        vkFreeCommandBuffers(
            device->device(),
            device->getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data()
        );
        commandBuffers.clear();
        LOG_DEBUG("Command buffers freed");
    }

    void Engine::recordCommandBuffer(int imageIndex) const {
        LOG_TRACE("Recording command buffer for imageIndex={}", imageIndex);
        static int frame = 0;
        frame = (frame + 1) % 100;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            LOG_FATAL("Failed to begin recording command buffer (imageIndex={})", imageIndex);
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
            LOG_FATAL("Failed to record command buffer (imageIndex={})", imageIndex);
            throw std::runtime_error("failed to record command buffer");
        }
        LOG_TRACE("Command buffer recorded for imageIndex={}", imageIndex);
    }

    void Engine::drawFrame() {
        LOG_TRACE("Drawing frame");
        uint32_t imageIndex;
        auto result = swapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            LOG_WARN("Swapchain out of date, recreating swapchain");
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            LOG_FATAL("Failed to acquire swapchain image: VkResult={}", static_cast<int>(result));
            throw std::runtime_error("failed to acquire swapchain image");
        }

        recordCommandBuffer(imageIndex);
        result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR
         || result == VK_SUBOPTIMAL_KHR
         || window->wasWindowResized()) {
            LOG_WARN("Swapchain out of date/suboptimal or window resized, recreating swapchain");
            window->resetWindowResizedFlag();
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS) {
            LOG_FATAL("Failed to present swapchain image: VkResult={}", static_cast<int>(result));
            throw std::runtime_error("failed to present swapchain image");
        }
        LOG_TRACE("Frame drawn successfully");
    }

} // namespace vkp::graphics
