#include "game.hpp"

void Game::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

Game::~Game() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Game::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Dig", nullptr, nullptr);
}

void Game::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffer();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffer();
    createSyncObjects();
}

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

void Game::createSyncObjects() {
    imageAvailableSemaphore = std::make_unique<vk::raii::Semaphore>(device->createSemaphore({}));
    renderFinishedSemaphore = std::make_unique<vk::raii::Semaphore>(device->createSemaphore({}));
    inFlightFence = std::make_unique<vk::raii::Fence>(device->createFence({vk::FenceCreateFlagBits::eSignaled}));
}