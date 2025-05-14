#pragma once

#include <vkp/window.h>
#include <vkp/graphics/device.h>
#include <vkp/graphics/swap_chain.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vulkan/vulkan.h>

namespace vkp {

    class ImGuiLayer {
    public:
        ImGuiLayer(Window& window,
                   vkp::graphics::Device& device,
                   vkp::graphics::SwapChain& swapChain,
                   VkRenderPass renderPass,
                   uint32_t subpass = 0);
        ~ImGuiLayer();

        void OnAttach();
        void OnDetach() const;
        void OnRender(VkCommandBuffer cmd);

    private:
        const float           StatsPos_x = 150.f;
        const float           StatsPos_y = 20.f;
        Window&               window_;
        vkp::graphics::Device&     device_;
        vkp::graphics::SwapChain&  swapChain_;
        VkRenderPass          renderPass_;
        uint32_t              subpass_;
        VkDescriptorPool      descriptorPool_;
        double   stats_last_update_time_   = 0.0;
        float    stats_fps_                = 0.0f;
        float    stats_frame_time_ms_      = 0.0f;
        float    stats_update_interval_    = 0.25f;
    };

} // namespace vkp
