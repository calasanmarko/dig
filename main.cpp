#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        return -1;
    }

    // Initialize Vulkan
    VkInstance instance;
    // Vulkan initialization code here

    // Use GLM
    glm::vec2 a(1.0f, 1.0f);
    glm::vec2 b(0.0f, 0.0f);
    glm::vec2 c = a + b;

    // Main loop
    while (true) {
        // Game loop code
    }

    // Cleanup
    // Vulkan and GLFW cleanup code here

    return 0;
}
