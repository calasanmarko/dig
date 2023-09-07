#include "game.hpp"

void Game::createSwapChain() {
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

vk::SurfaceFormatKHR Game::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Game::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Game::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    vk::Extent2D actualExtent;
    actualExtent.width = std::clamp(static_cast<uint32_t>(WIDTH), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(static_cast<uint32_t>(HEIGHT), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
}

void Game::createFramebuffers() {
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