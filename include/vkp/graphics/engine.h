#pragma once

#include <vkp/window.h>
#include "device.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "../imgui_layer.h"

#include <memory>
#include <vector>

namespace vkp::graphics {

    struct engine_conf {
        int start_pos_x;
        int start_pos_y;
        int start_width;
        int start_height;
        const char* name;
    };
    class Engine {
    public:
        static constexpr int WIDTH  = 1280;
        static constexpr int HEIGHT = 720;

        Engine();
        ~Engine();

        bool init(const engine_conf& config); 
        bool run();

    private:
        int   width_{ 0 };
        int   height_{ 0 };

        void createPipelineLayout();
        void recreateSwapChain();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void recordCommandBuffer(int imageIndex) const;
        void drawFrame();
        void shutdown() const;

        Window window{ WIDTH, HEIGHT, "Delta Trick Demo", 500, 500 };
        vkp::graphics::Device device{ window };
        std::unique_ptr<graphics::SwapChain>    swapChain;
        std::unique_ptr<graphics::Pipeline>     pipeline;
        VkPipelineLayout                        pipelineLayout{};

        std::vector<VkCommandBuffer>            commandBuffers;
        std::unique_ptr<vkp::ImGuiLayer>        imguiLayer;
    };

} // namespace vkp::graphics
