#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace vkp {
    class Window {
    public:
        Window(int width, int height, const std::string& title, int pos_x, int pos_y);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        [[nodiscard]] GLFWwindow* handle() const { return window; }
        [[nodiscard]] bool shouldClose() const;
        [[nodiscard]] VkExtent2D getExtent() const { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

        void pollEvents() const;
        void createSurface(VkInstance instance, VkSurfaceKHR* surface) const;
        bool wasWindowResized() const { return framebufferResized; }
        void resetWindowResizedFlag() { framebufferResized = false; }

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
        void init() const;

        int width;
        int height;
        std::string title;
        GLFWwindow* window;
        bool framebufferResized = false;
    };
}