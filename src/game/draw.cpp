#include "game.hpp"

void Game::recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex) {
    commandBuffer.begin({});

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = **renderPass;
    renderPassInfo.framebuffer = *swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D();
    renderPassInfo.renderArea.extent = swapChainExtent;

    vk::ClearValue clearColor({0.1f, 0.1f, 0.1f, 1.0f});
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **graphicsPipeline);

    commandBuffer.bindVertexBuffers(0, {**vertexBuffer.buffer}, {0});
    commandBuffer.bindIndexBuffer(**indexBuffer.buffer, 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0, {**descriptorSet}, nullptr);

    commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void Game::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }
    device->waitIdle();
}

void Game::drawFrame() {
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

void Game::updateUniformBuffer() {
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