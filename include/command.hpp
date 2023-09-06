#pragma once
#include "includes.hpp"

vk::raii::CommandBuffer beginSingleTimeCommands(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool) {
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = *commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    auto commandBuffer = std::move(device.allocateCommandBuffers(allocInfo)[0]);
    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    return commandBuffer;
}

void endSingleTimeCommands(const vk::raii::Queue& presentQueue, const vk::raii::CommandBuffer& commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffer;

    presentQueue.submit(submitInfo);
    presentQueue.waitIdle();
}

void transitionImageLayout(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool, const vk::raii::Queue& presentQueue, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

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
    endSingleTimeCommands(presentQueue, commandBuffer);
}

void copyBufferToImage(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool, const vk::raii::Queue& presentQueue, const vk::raii::Buffer& buffer, vk::Image image, uint32_t width, uint32_t height) {
    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

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
    endSingleTimeCommands(presentQueue, commandBuffer);
}

void copyBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool, const vk::raii::Queue& presentQueue, const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size) {
    auto commandBuffer = beginSingleTimeCommands(device, commandPool);

    vk::BufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, {copyRegion});
    endSingleTimeCommands(presentQueue, commandBuffer);
}