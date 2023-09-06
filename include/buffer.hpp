#pragma once
#include "includes.hpp"

uint32_t findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memoryProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

struct BufferWithMemory {
    std::unique_ptr<vk::raii::Buffer> buffer;
    std::unique_ptr<vk::raii::DeviceMemory> memory;

    BufferWithMemory() = default;

    BufferWithMemory(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        
        buffer = std::make_unique<vk::raii::Buffer>(device.createBuffer(bufferInfo));
        auto memoryRequirements = buffer->getMemoryRequirements();

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties);

        memory = std::make_unique<vk::raii::DeviceMemory>(device, allocInfo);
        buffer->bindMemory(**memory, 0);
    }
};