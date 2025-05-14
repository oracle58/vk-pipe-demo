#include <vkp/gui/imgui_layer.h>
#include <vkp/logger.h>

namespace vkp {

ImGuiLayer::ImGuiLayer(
    Window& window,
    vkp::graphics::Device& device,
    vkp::graphics::SwapChain& swapChain,
    const VkRenderPass renderPass,
    const uint32_t subpass)
    : window_(window)
    , device_(device)
    , swapChain_(swapChain)
    , renderPass_(renderPass)
    , subpass_(subpass)
    , descriptorPool_(VK_NULL_HANDLE)
{
}

ImGuiLayer::~ImGuiLayer() = default;

void ImGuiLayer::OnAttach() {
    // Descriptor pool for ImGui: only combined image samplers, large count for safety.
    constexpr VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    pool_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets       = 1000;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes    = pool_sizes;
    vkCreateDescriptorPool(device_.device(), &pool_info, nullptr, &descriptorPool_);

    // ImGui context & style setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // GLFW backend for input handling
    ImGui_ImplGlfw_InitForVulkan(window_.handle(), true);

    // Vulkan backend: fill out all required info for ImGui's renderer
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance        = device_.getInstance();
    init_info.PhysicalDevice  = device_.getPhysicalDevice();
    init_info.Device          = device_.device();
    init_info.QueueFamily     = device_.getGraphicsQueueFamilyIndex();
    init_info.Queue           = device_.getGraphicsQueue();
    init_info.PipelineCache   = VK_NULL_HANDLE;
    init_info.DescriptorPool  = descriptorPool_;
    init_info.Subpass         = subpass_;
    init_info.MinImageCount   = 2;
    init_info.ImageCount      = static_cast<uint32_t>(swapChain_.imageCount());
    init_info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator       = nullptr;
    init_info.RenderPass      = renderPass_;

    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiLayer::OnDetach() const {
    // Order is important: destroy ImGui resources before Vulkan pool/context
    vkDestroyDescriptorPool(device_.device(), descriptorPool_, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::OnRender(VkCommandBuffer cmd) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    // Only update stats at a fixed interval to avoid UI flicker
    if (double now = ImGui::GetTime(); now - stats_last_update_time_ >= stats_update_interval_) {
        stats_fps_           = io.Framerate;
        stats_frame_time_ms_ = io.DeltaTime * 1000.0f;
        stats_last_update_time_ = now;
    }

    // Overlay stats window in top-right, always visible, no interaction
    auto [width, height] = swapChain_.getSwapChainExtent();
    ImGui::SetNextWindowPos(
        ImVec2(static_cast<float>(width) - StatsPos_x, StatsPos_y),
        ImGuiCond_Always
    );
    ImGui::Begin(
        "Stats",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoInputs
    );

    ImGui::Text("FPS: %.f", stats_fps_);
    ImGui::Text("FrameTime: %.1f ms", stats_frame_time_ms_);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

} // namespace vkp
