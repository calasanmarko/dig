#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentationFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentationFamily.has_value();
    }
};

class Game {
    public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
    }

    ~Game() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    private:
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
    std::unique_ptr<vk::raii::DeviceMemory> vertexBufferMemory, indexBufferMemory, uniformBufferMemory, textureImageMemory;
    std::unique_ptr<vk::raii::Buffer> vertexBuffer, indexBuffer, uniformBuffer;
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

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Dig", nullptr, nullptr);
    }

    void initVulkan() {
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

    void createInstance() {
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

    void createSurface() {
        VkSurfaceKHR vkSurface;
        VkInstance nativeInstance = static_cast<VkInstance>(**instance);

        auto res = glfwCreateWindowSurface(nativeInstance, window, nullptr, &vkSurface);
        
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }

        surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, vkSurface);
    }

    void pickPhysicalDevice() {
        auto devices = (**instance).enumeratePhysicalDevices();
        
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(*instance, device);
                return;
            }
        }
        
        throw std::runtime_error("Failed to find suitable GPU");
    }

    bool isDeviceSuitable(const vk::PhysicalDevice& device) {
        return findQueueFamilies(device).isComplete();
    }

    QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device) {
        QueueFamilyIndices indices;
        
        auto queueFamilies = device.getQueueFamilyProperties();

        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = i;
                indices.presentationFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
        }

        return indices;
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(**physicalDevice);
        float queuePriority = 1.0f;

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentationFamily.value()};

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures;
        deviceFeatures.samplerAnisotropy = vk::True;

        vk::DeviceCreateInfo createInfo;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.enabledExtensionCount = deviceExtensions.size();
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        device = std::make_unique<vk::raii::Device>(*physicalDevice, createInfo);
        presentQueue = std::make_unique<vk::raii::Queue>(*device, indices.presentationFamily.value(), 0);
    }

    void createSwapChain() {
        auto surfaceCapabilities = physicalDevice->getSurfaceCapabilitiesKHR(**surface);

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(physicalDevice->getSurfaceFormatsKHR(**surface));
        vk::PresentModeKHR presentationMode = chooseSwapPresentMode(physicalDevice->getSurfacePresentModesKHR(**surface));
        vk::Extent2D extent = chooseSwapExtent(surfaceCapabilities);

        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
            imageCount = surfaceCapabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo = {};
        createInfo.surface = **surface;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.presentMode = presentationMode;
        createInfo.imageExtent = extent;
        createInfo.minImageCount = imageCount;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.preTransform = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.oldSwapchain = nullptr;

        swapChainImageFormat = createInfo.imageFormat;
        swapChainExtent = createInfo.imageExtent;

        swapChain = std::make_unique<vk::raii::SwapchainKHR>(*device, createInfo);
        swapChainImages = swapChain->getImages();
    }

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }
    
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        vk::Extent2D actualExtent;
        actualExtent.width = std::clamp(static_cast<uint32_t>(WIDTH), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(static_cast<uint32_t>(HEIGHT), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }

    vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format) {
        vk::ImageViewCreateInfo createInfo;
        createInfo.image = image;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = format;

        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;

        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.levelCount = 1;

        return vk::raii::ImageView(*device, createInfo);
    }

    void createImageViews() {
        swapChainImageViews.clear();
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews.push_back(createImageView(swapChainImages[i], swapChainImageFormat));
        }
    }

    void createRenderPass() {
        vk::AttachmentDescription colorAttachment;
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass;
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency subpassDependency;
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        subpassDependency.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
        subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

        vk::RenderPassCreateInfo renderPassInfo;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDependency;
        
        renderPass = std::make_unique<vk::raii::RenderPass>(*device, renderPassInfo);
    }

    void createDescriptorSetLayout() {
        vk::DescriptorSetLayoutBinding uboLayoutBinding;
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

        vk::DescriptorSetLayoutBinding samplerLayoutBinding;
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        
        descriptorSetLayout = std::make_unique<vk::raii::DescriptorSetLayout>(*device, layoutInfo);
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("shaders/build/vert.spv");
        auto fragShaderCode = readFile("shaders/build/frag.spv");

        auto vertShaderModule = createShaderModule(vertShaderCode);
        auto fragShaderModule = createShaderModule(fragShaderCode);

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = *vertShaderModule;
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = *fragShaderModule;
        fragShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssemblyInfo.primitiveRestartEnable = vk::False;

        vk::Viewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor;
        scissor.offset = vk::Offset2D();
        scissor.extent = swapChainExtent;

        vk::PipelineViewportStateCreateInfo viewportInfo;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &viewport;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &scissor;

        vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
        rasterizerInfo.depthClampEnable = vk::False;
        rasterizerInfo.rasterizerDiscardEnable = vk::False;
        rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
        rasterizerInfo.lineWidth = 1.0f;
        rasterizerInfo.cullMode = vk::CullModeFlagBits::eNone;
        rasterizerInfo.frontFace = vk::FrontFace::eCounterClockwise;
        rasterizerInfo.depthBiasEnable = vk::False;

        vk::PipelineMultisampleStateCreateInfo multisamplingInfo;
        multisamplingInfo.sampleShadingEnable = vk::False;
        multisamplingInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = vk::True;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
        colorBlendInfo.logicOpEnable = vk::False;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &colorBlendAttachment;

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &**descriptorSetLayout;
        
        pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(*device, pipelineLayoutInfo);

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &rasterizerInfo;
        pipelineInfo.pMultisampleState = &multisamplingInfo;
        pipelineInfo.pColorBlendState = &colorBlendInfo;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = **pipelineLayout;
        pipelineInfo.renderPass = **renderPass;
        pipelineInfo.subpass = 0;
        
        graphicsPipeline = std::make_unique<vk::raii::Pipeline>(*device, nullptr, pipelineInfo);
    }

    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        return device->createShaderModule(createInfo);
    }

    void createFramebuffers() {
        swapChainFramebuffers.clear();
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            vk::ImageView attachments[] = {*swapChainImageViews[i]};

            vk::FramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.renderPass = **renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;
            
            swapChainFramebuffers.emplace_back(*device, framebufferInfo);
        }
    }

    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(**physicalDevice);

        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        
        commandPool = std::make_unique<vk::raii::CommandPool>(*device, poolInfo);
    }

    void createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load("textures/cat.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image");
        }

        std::unique_ptr<vk::raii::DeviceMemory> stagingBufferMemory;
        auto stagingBuffer = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &stagingBufferMemory);

        void *data = stagingBufferMemory->mapMemory(0, imageSize);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        stagingBufferMemory->unmapMemory();

        stbi_image_free(pixels);

        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = vk::Format::eR8G8B8A8Srgb;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        
        textureImage = std::make_unique<vk::raii::Image>(*device, imageInfo);

        auto memoryRequirements = textureImage->getMemoryRequirements();

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        
        textureImageMemory = std::make_unique<vk::raii::DeviceMemory>(*device, allocInfo);

        textureImage->bindMemory(**textureImageMemory, 0);

        transitionImageLayout(**textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        copyBufferToImage(stagingBuffer, **textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(**textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
        vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::ImageMemoryBarrier barrier;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else {
            throw std::runtime_error("Unsupported layout transition");
        }

        commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
        endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(const vk::raii::Buffer& buffer, vk::Image image, uint32_t width, uint32_t height) {
        vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferImageCopy region;
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageOffset = vk::Offset3D();
        region.imageExtent = vk::Extent3D(width, height, 1);
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageSubresource.mipLevel = 0;

        commandBuffer.copyBufferToImage(*buffer, image, vk::ImageLayout::eTransferDstOptimal, {region});
        endSingleTimeCommands(commandBuffer);
    }

    vk::raii::Buffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, std::unique_ptr<vk::raii::DeviceMemory>* bufferMemory) {
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        
        auto buffer = device->createBuffer(bufferInfo);
        auto memoryRequirements = buffer.getMemoryRequirements();

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

        *bufferMemory = std::make_unique<vk::raii::DeviceMemory>(*device, allocInfo);
        buffer.bindMemory(***bufferMemory, 0);

        return buffer;
    }

    vk::raii::CommandBuffer beginSingleTimeCommands() {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.commandPool = **commandPool;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;

        auto commandBuffer = std::move(device->allocateCommandBuffers(allocInfo)[0]);
        commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

        return commandBuffer;
    }

    void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) {
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &*commandBuffer;

        presentQueue->submit(submitInfo);
        presentQueue->waitIdle();
    }

    void copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size) {
        auto commandBuffer = beginSingleTimeCommands();

        vk::BufferCopy copyRegion;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, {copyRegion});
        endSingleTimeCommands(commandBuffer);
    }

    void createTextureImageView() {
        textureImageView = std::make_unique<vk::raii::ImageView>(createImageView(**textureImage, vk::Format::eR8G8B8A8Srgb));
    }

    void createTextureSampler() {
        auto properties = physicalDevice->getProperties();

        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.magFilter = vk::Filter::eNearest;
        samplerInfo.minFilter = vk::Filter::eNearest;

        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

        samplerInfo.anisotropyEnable = vk::True;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;

        samplerInfo.unnormalizedCoordinates = vk::False;

        samplerInfo.compareEnable = vk::False;
        samplerInfo.compareOp = vk::CompareOp::eAlways;

        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = 0;

        textureSampler = std::make_unique<vk::raii::Sampler>(device->createSampler(samplerInfo));
    }

    void createVertexBuffer() {
        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        std::unique_ptr<vk::raii::DeviceMemory> stagingBufferMemory;
        auto stagingBuffer = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &stagingBufferMemory);

        void *data = stagingBufferMemory->mapMemory(0, bufferSize);
        memcpy(data, vertices.data(), bufferSize);
        stagingBufferMemory->unmapMemory();

        vertexBuffer = std::make_unique<vk::raii::Buffer>(createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, &vertexBufferMemory));
        copyBuffer(stagingBuffer, *vertexBuffer, bufferSize);
    }

    void createIndexBuffer() {
        vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        std::unique_ptr<vk::raii::DeviceMemory> stagingBufferMemory;
        auto stagingBuffer = createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &stagingBufferMemory);

        void *data = stagingBufferMemory->mapMemory(0, bufferSize);
        memcpy(data, indices.data(), bufferSize);
        stagingBufferMemory->unmapMemory();

        indexBuffer = std::make_unique<vk::raii::Buffer>(createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, &indexBufferMemory));
        copyBuffer(stagingBuffer, *indexBuffer, bufferSize);
    }

    void createUniformBuffer() {
        vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffer = std::make_unique<vk::raii::Buffer>(createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &uniformBufferMemory));
        uniformBufferMapped = uniformBufferMemory->mapMemory(0, bufferSize);
    }

    void createDescriptorPool() {
        std::array<vk::DescriptorPoolSize, 2> poolSizes;
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = 1;
        poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
        poolSizes[1].descriptorCount = 1;

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 1;
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

        descriptorPool = std::make_unique<vk::raii::DescriptorPool>(*device, poolInfo);
    }

    void createDescriptorSets() {
        vk::DescriptorSetAllocateInfo allocInfo;
        allocInfo.descriptorPool = **descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &**descriptorSetLayout;

        descriptorSet = std::make_unique<vk::raii::DescriptorSet>(*device, (**device).allocateDescriptorSets(allocInfo)[0], static_cast<VkDescriptorPool>(**descriptorPool));

        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = **uniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        vk::DescriptorImageInfo imageInfo;
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = **textureImageView;
        imageInfo.sampler = **textureSampler;

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};

        descriptorWrites[0].dstSet = **descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].dstSet = **descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        device->updateDescriptorSets(descriptorWrites, nullptr);
    }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
        auto memoryProperties = physicalDevice->getMemoryProperties();

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type");
    }

    void createCommandBuffer() {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.commandPool = **commandPool;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;

        auto vkBuffer = static_cast<VkCommandBuffer>((**device).allocateCommandBuffers(allocInfo)[0]);
        auto vkPool = static_cast<VkCommandPool>(**commandPool);
        
        commandBuffer = std::make_unique<vk::raii::CommandBuffer>(*device, vkBuffer, vkPool);
    }

    void createSyncObjects() {
        imageAvailableSemaphore = std::make_unique<vk::raii::Semaphore>(device->createSemaphore({}));
        renderFinishedSemaphore = std::make_unique<vk::raii::Semaphore>(device->createSemaphore({}));
        inFlightFence = std::make_unique<vk::raii::Fence>(device->createFence({vk::FenceCreateFlagBits::eSignaled}));
    }

    void recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex) {
        commandBuffer.begin({});

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.renderPass = **renderPass;
        renderPassInfo.framebuffer = *swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = vk::Offset2D();
        renderPassInfo.renderArea.extent = swapChainExtent;

        vk::ClearValue clearColor({0.2f, 0.2f, 0.0f, 1.0f});
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **graphicsPipeline);

        commandBuffer.bindVertexBuffers(0, {**vertexBuffer}, {0});
        commandBuffer.bindIndexBuffer(**indexBuffer, 0, vk::IndexType::eUint16);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0, {**descriptorSet}, nullptr);

        commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        device->waitIdle();
    }

    void drawFrame() {
        device->waitForFences({**inFlightFence}, vk::True, UINT64_MAX);
        device->resetFences({**inFlightFence});

        uint32_t imageIndex = swapChain->acquireNextImage(UINT64_MAX, **imageAvailableSemaphore).second;

        updateUniformBuffer();

        commandBuffer->reset();
        recordCommandBuffer(*commandBuffer, imageIndex);

        vk::Semaphore waitSemaphores[] = {**imageAvailableSemaphore};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::Semaphore signalSemaphores[] = {**renderFinishedSemaphore};

        vk::SubmitInfo submitInfo = {};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &**commandBuffer;

        presentQueue->submit({submitInfo}, **inFlightFence);

        vk::SwapchainKHR swapChains[] = {**swapChain};
        vk::PresentInfoKHR presentInfo = {};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        presentQueue->presentKHR(presentInfo);
    }

    void updateUniformBuffer() {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo = {};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(30.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
    }

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
};

int main() {
    Game game;

    try {
        game.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
