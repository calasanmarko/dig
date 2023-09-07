#include "game.hpp"

void Game::createDescriptorPool() {
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

void Game::createDescriptorSets() {
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = **descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &**descriptorSetLayout;

    descriptorSet = std::make_unique<vk::raii::DescriptorSet>(*device, (**device).allocateDescriptorSets(allocInfo)[0], static_cast<VkDescriptorPool>(**descriptorPool));

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = **uniformBuffer.buffer;
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