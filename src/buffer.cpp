#include "includes.hpp"
#include "game.hpp"

BufferWithMemory::BufferWithMemory(Game* game, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
    vk::BufferCreateInfo bufferInfo;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    buffer = std::make_unique<vk::raii::Buffer>(game->device->createBuffer(bufferInfo));
    auto memoryRequirements = buffer->getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = game->findMemoryType(*game->physicalDevice, memoryRequirements.memoryTypeBits, properties);

    memory = std::make_unique<vk::raii::DeviceMemory>(*game->device, allocInfo);
    buffer->bindMemory(**memory, 0);
}

BufferWithMemory Game::createStagedBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, void* data) {
    BufferWithMemory stagingBuffer(this, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *stagingData = stagingBuffer.memory->mapMemory(0, size);
    memcpy(stagingData, data, size);
    stagingBuffer.memory->unmapMemory();

    auto result = BufferWithMemory(this, size, vk::BufferUsageFlagBits::eTransferDst | usage, properties);
    copyBuffer(*stagingBuffer.buffer, *result.buffer, size);

    return result;
}

void Game::createVertexBuffer() {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    vertexBuffer = createStagedBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, (void*)vertices.data());
}

void Game::createIndexBuffer() {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    indexBuffer = createStagedBuffer(bufferSize, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, (void*)indices.data());
}

void Game::createUniformBuffer() {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffer = BufferWithMemory(this, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    uniformBufferMapped = uniformBuffer.memory->mapMemory(0, bufferSize);
}

uint32_t Game::findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memoryProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}