#pragma once
#include "includes.hpp"

class Game;

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    // --- render.cpp ---
    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentationFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentationFamily.has_value();
    }
};

struct BufferWithMemory {
    std::unique_ptr<vk::raii::Buffer> buffer;
    std::unique_ptr<vk::raii::DeviceMemory> memory;

    BufferWithMemory() = default;

    // --- buffer.cpp ---
    BufferWithMemory(Game* game, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
};

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_monitor"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

class Game {
    public:
    GLFWwindow* window;
    vk::raii::Context context;
    std::unique_ptr<vk::raii::Instance> instance;
    std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;
    std::unique_ptr<vk::raii::Device> device;
    std::unique_ptr<vk::raii::SurfaceKHR> surface;
    std::unique_ptr<vk::raii::Queue> presentQueue;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::unique_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<vk::raii::DeviceMemory> textureImageMemory;
    BufferWithMemory vertexBuffer, indexBuffer, uniformBuffer;
    void *uniformBufferMapped;
    std::unique_ptr<vk::raii::Image> textureImage;
    std::unique_ptr<vk::raii::ImageView> textureImageView;
    std::unique_ptr<vk::raii::Sampler> textureSampler;
    std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;
    std::unique_ptr<vk::raii::DescriptorSet> descriptorSet;
    std::unique_ptr<vk::raii::SwapchainKHR> swapChain;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::raii::ImageView> swapChainImageViews;
    std::unique_ptr<vk::raii::RenderPass> renderPass;
    std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
    std::unique_ptr<vk::raii::Pipeline> graphicsPipeline;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers;
    std::unique_ptr<vk::raii::CommandPool> commandPool;
    std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
    std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore, renderFinishedSemaphore;
    std::unique_ptr<vk::raii::Fence> inFlightFence;

    // --- lifecycle.cpp ---
    void run();
    ~Game();
    void initWindow();
    void initVulkan();

    // --- instance.cpp ---
    void createInstance();
    void createSurface();

    // --- device.cpp ---
    void pickPhysicalDevice();
    bool isDeviceSuitable(const vk::PhysicalDevice& device);
    QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device);
    void createLogicalDevice();

    // --- swapchain.cpp ---
    void createSwapChain();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    void createFramebuffers();

    // --- image_view.cpp ---
    vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format);
    void createImageViews();

    // --- render.cpp
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code);

    // --- texture.cpp ---
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    std::vector<char> readFile(const std::string& filename);

    // --- shader_buffer.cpp ---
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();

    // --- descriptor.cpp ---
    void createDescriptorPool();
    void createDescriptorSets();

    // --- sync.cpp ---
    void createSyncObjects();

    // --- buffer.cpp ---
    uint32_t findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    // --- command.cpp ---
    void createCommandPool();
    void createCommandBuffer();
    vk::raii::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer);
    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void copyBufferToImage(const vk::raii::Buffer& buffer, vk::Image image, uint32_t width, uint32_t height);
    void copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size);

    // --- draw.cpp ---
    void recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex);
    void mainLoop();
    void drawFrame();
    void updateUniformBuffer();
};