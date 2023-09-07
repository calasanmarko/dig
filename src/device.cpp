#include "game.hpp"

void Game::pickPhysicalDevice() {
    auto devices = (**instance).enumeratePhysicalDevices();
    
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(*instance, device);
            return;
        }
    }
    
    throw std::runtime_error("Failed to find suitable GPU");
}

bool Game::isDeviceSuitable(const vk::PhysicalDevice& device) {
    return findQueueFamilies(device).isComplete();
}

QueueFamilyIndices Game::findQueueFamilies(const vk::PhysicalDevice& device) {
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

void Game::createLogicalDevice() {
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