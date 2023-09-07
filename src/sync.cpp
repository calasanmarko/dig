#include "game.hpp"

void Game::createSyncObjects() {
    imageAvailableSemaphore = std::make_unique<vk::raii::Semaphore>(device->createSemaphore({}));
    renderFinishedSemaphore = std::make_unique<vk::raii::Semaphore>(device->createSemaphore({}));
    inFlightFence = std::make_unique<vk::raii::Fence>(device->createFence({vk::FenceCreateFlagBits::eSignaled}));
}