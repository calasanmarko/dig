cmake_minimum_required(VERSION 3.10)

project(Dig)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Stb REQUIRED)

add_executable(Dig main.cpp)

target_link_libraries(Dig Vulkan::Vulkan)
target_link_libraries(Dig glm::glm)
target_link_libraries(Dig glfw)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)