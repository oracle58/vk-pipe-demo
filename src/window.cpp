#include <vkp/window.h>
#include <vkp/logger.h>

namespace vkp {
    Window::Window(int width, int height, const std::string& title, int pos_x, int pos_y)
        : width(width), height(height), title(title), window(nullptr) {
        if (!glfwInit()) {
            LOG_FATAL("Failed to initialize GLFW.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            LOG_FATAL("Failed to create GLFW window.");
            glfwTerminate();
        }

        init();
    }

    Window::~Window() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto window_ = static_cast<Window *>(glfwGetWindowUserPointer(window));
        window_->framebufferResized = true;
        window_->width = width;
        window_->height = height;
    }

    void Window::init() const {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        int x, y, w, h;
        glfwGetMonitorWorkarea(primary, &x, &y, &w, &h);
        glfwSetWindowPos(window, x + (w - width) / 2, y + (h - height) / 2);
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(window);
    }

    void Window::pollEvents() const {
        glfwPollEvents();
    }

    void Window::createSurface(VkInstance instance, VkSurfaceKHR* surface) const {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }
}