#pragma once

#include "device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace vkp::graphics {

class SwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  SwapChain(Device &deviceRef, VkExtent2D windowExtent);
  SwapChain(
      Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);

  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int index) const { return swapChainFramebuffers[index]; }
  VkRenderPass getRenderPass() const { return renderPass; }
  VkImageView getImageView(int index) const { return swapChainImageViews[index]; }
  size_t imageCount() const { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
  uint32_t width() const { return swapChainExtent.width; }
  uint32_t height() const { return swapChainExtent.height; }

  float extentAspectRatio() const {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat() const;

  VkResult acquireNextImage(uint32_t *imageIndex) const;
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, const uint32_t *imageIndex);

 private:
  void init();
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes) const;
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  Device &device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<SwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};

}  // namespace vkp::graphics