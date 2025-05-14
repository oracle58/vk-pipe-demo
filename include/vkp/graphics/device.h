#pragma once

#include <vkp/gui/window.h>
#include <vector>

namespace vkp::graphics {

 // Holds all swap chain support details for a physical device.
 struct SwapChainSupportDetails {
   VkSurfaceCapabilitiesKHR capabilities;
   std::vector<VkSurfaceFormatKHR> formats;
   std::vector<VkPresentModeKHR> presentModes;
 };

 // Stores queue family indices and their validity.
 struct QueueFamilyIndices {
   uint32_t graphicsFamily{};
   uint32_t presentFamily{};
   bool graphicsFamilyHasValue = false;
   bool presentFamilyHasValue = false;
   // Returns true if both graphics and present families are found.
   [[nodiscard]] bool isComplete() const { return graphicsFamilyHasValue && presentFamilyHasValue; }
 };

 // Encapsulates Vulkan device and related resource management.
 class Device {
  public:
 #ifdef NDEBUG
   const bool enableValidationLayers = false;
 #else
   const bool enableValidationLayers = true;
 #endif

   explicit Device(Window &window); // Initializes Vulkan device for the given window.
   ~Device();

   // Not copyable or movable: device resources must not be duplicated.
   Device(const Device &) = delete;
   void operator=(const Device &) = delete;
   Device(Device &&) = delete;
   Device &operator=(Device &&) = delete;

   // Accessors for Vulkan handles.
   [[nodiscard]] VkCommandPool getCommandPool() const { return commandPool; }
   [[nodiscard]] VkDevice device() const { return device_; }
   [[nodiscard]] VkSurfaceKHR surface() const { return surface_; }
   [[nodiscard]] VkQueue graphicsQueue() const { return graphicsQueue_; }
   [[nodiscard]] VkQueue presentQueue() const { return presentQueue_; }

   // Swap chain and memory helpers.
   [[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const { return querySwapChainSupport(physicalDevice); }
   [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
   [[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(physicalDevice); }
   [[nodiscard]] VkFormat findSupportedFormat(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

   // Buffer and image helpers for resource uploads and staging.
   void createBuffer(
       VkDeviceSize size,
       VkBufferUsageFlags usage,
       VkMemoryPropertyFlags properties,
       VkBuffer &buffer,
       VkDeviceMemory &bufferMemory) const;
   [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;
   void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;
   void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
   void copyBufferToImage(
       VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;

   void createImageWithInfo(
       const VkImageCreateInfo &imageInfo,
       VkMemoryPropertyFlags properties,
       VkImage &image,
       VkDeviceMemory &imageMemory) const;

   // Additional accessors.
   [[nodiscard]] VkInstance getInstance() const;
   [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const;
   [[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const;
   [[nodiscard]] VkQueue getGraphicsQueue() const;

   VkPhysicalDeviceProperties properties; // Physical device properties (limits, features, etc.)

  private:
   // Vulkan setup and teardown routines.
   void createInstance();
   void setupDebugMessenger();
   void createSurface();
   void pickPhysicalDevice();
   void createLogicalDevice();
   void createCommandPool();

   // Device suitability and extension checks.
   bool isDeviceSuitable(VkPhysicalDevice device) const;
   std::vector<const char *> getRequiredExtensions();
   [[nodiscard]] bool checkValidationLayerSupport() const;
   QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

   // Debug and swap chain helpers.
   static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
   void hasGflwRequiredInstanceExtensions();
   bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
   SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

   // Vulkan handles and state.
   VkInstance instance;
   VkDebugUtilsMessengerEXT debugMessenger;
   VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
   Window &window;
   VkCommandPool commandPool;

   VkDevice device_;
   VkSurfaceKHR surface_;
   VkQueue graphicsQueue_;
   VkQueue presentQueue_;

   // Required validation layers and device extensions.
   const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
   const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
 };

}  // namespace vkp::graphics