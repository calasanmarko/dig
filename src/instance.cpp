#include "game.hpp"

void Game::createInstance() {
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Dig";
    appInfo.applicationVersion = vk::makeApiVersion(1, 0, 0, 0);
    appInfo.pEngineName = "Dig Engine";
    appInfo.engineVersion = vk::makeApiVersion(1, 0, 0, 0);
    appInfo.apiVersion = vk::ApiVersion13;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
    
    instance = std::make_unique<vk::raii::Instance>(context, createInfo);
}

void Game::createSurface() {
    VkSurfaceKHR vkSurface;
    VkInstance nativeInstance = static_cast<VkInstance>(**instance);

    auto res = glfwCreateWindowSurface(nativeInstance, window, nullptr, &vkSurface);
    
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, vkSurface);
}