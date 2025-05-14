#pragma once

#include <vkp/window.h>

#include <string>
#include <vector>

namespace vkp::graphics {

 struct SwapChainSupportDetails {
   VkSurfaceCapabilitiesKHR capabilities;
   std::vector<VkSurfaceFormatKHR> formats;
   std::vector<VkPresentModeKHR> presentModes;
 };

 struct QueueFamilyIndices {
   uint32_t graphicsFamily{};
   uint32_t presentFamily{};
   bool graphicsFamilyHasValue = false;
   bool presentFamilyHasValue = false;
   [[nodiscard]] bool isComplete() const { return graphicsFamilyHasValue && presentFamilyHasValue; }
 };

 class Device {
  public:
 #ifdef NDEBUG
   const bool enableValidationLayers = false;
 #else
   const bool enableValidationLayers = true;
 #endif

   explicit Device(Window &window);
   ~Device();

   // Not copyable or movable
   Device(const Device &) = delete;
   void operator=(const Device &) = delete;
   Device(Device &&) = delete;
   Device &operator=(Device &&) = delete;

   [[nodiscard]] VkCommandPool getCommandPool() const { return commandPool; }
   [[nodiscard]] VkDevice device() const { return device_; }
   [[nodiscard]] VkSurfaceKHR surface() const { return surface_; }
   [[nodiscard]] VkQueue graphicsQueue() const { return graphicsQueue_; }
   [[nodiscard]] VkQueue presentQueue() const { return presentQueue_; }

   SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
   uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
   QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
   VkFormat findSupportedFormat(
       const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

   // Buffer Helper Functions
   void createBuffer(
       VkDeviceSize size,
       VkBufferUsageFlags usage,
       VkMemoryPropertyFlags properties,
       VkBuffer &buffer,
       VkDeviceMemory &bufferMemory) const;
   VkCommandBuffer beginSingleTimeCommands() const;
   void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;
   void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
   void copyBufferToImage(
       VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const const;

   void createImageWithInfo(
       const VkImageCreateInfo &imageInfo,
       VkMemoryPropertyFlags properties,
       VkImage &image,
       VkDeviceMemory &imageMemory) const;

   VkInstance getInstance() const;

   VkPhysicalDevice getPhysicalDevice() const;

   uint32_t getGraphicsQueueFamilyIndex() const;

   VkQueue getGraphicsQueue() const;

   VkExtent2D surfaceExtent() const;

   VkPhysicalDeviceProperties properties;

  private:
   void createInstance();
   void setupDebugMessenger();
   void createSurface();
   void pickPhysicalDevice();
   void createLogicalDevice();
   void createCommandPool();

   // helper functions
   bool isDeviceSuitable(VkPhysicalDevice device);
   std::vector<const char *> getRequiredExtensions();
   [[nodiscard]] bool checkValidationLayerSupport() const;
   QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
   void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) const;
   void hasGflwRequiredInstanceExtensions();
   bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
   SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

   VkInstance instance;
   VkDebugUtilsMessengerEXT debugMessenger;
   VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
   Window &window;
   VkCommandPool commandPool;

   VkDevice device_;
   VkSurfaceKHR surface_;
   VkQueue graphicsQueue_;
   VkQueue presentQueue_;

   const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
   const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
 };

 }  // namespace vkp::graphics