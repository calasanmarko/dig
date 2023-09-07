#include "game.hpp"

void Game::createVertexBuffer() {
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    BufferWithMemory stagingBuffer(this, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *data = stagingBuffer.memory->mapMemory(0, bufferSize);
    memcpy(data, vertices.data(), bufferSize);
    stagingBuffer.memory->unmapMemory();

    vertexBuffer = BufferWithMemory(this, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
    copyBuffer(*stagingBuffer.buffer, *vertexBuffer.buffer, bufferSize);
}

void Game::createIndexBuffer() {
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    BufferWithMemory stagingBuffer(this, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *data = stagingBuffer.memory->mapMemory(0, bufferSize);
    memcpy(data, indices.data(), bufferSize);
    stagingBuffer.memory->unmapMemory();

    indexBuffer = BufferWithMemory(this, bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
    copyBuffer(*stagingBuffer.buffer, *indexBuffer.buffer, bufferSize);
}

void Game::createUniformBuffer() {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffer = BufferWithMemory(this, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    uniformBufferMapped = uniformBuffer.memory->mapMemory(0, bufferSize);
}