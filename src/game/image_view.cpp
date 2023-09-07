#include "game.hpp"

vk::raii::ImageView Game::createImageView(const vk::Image& image, vk::Format format) {
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

void Game::createImageViews() {
    swapChainImageViews.clear();
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews.push_back(createImageView(swapChainImages[i], swapChainImageFormat));
    }
}