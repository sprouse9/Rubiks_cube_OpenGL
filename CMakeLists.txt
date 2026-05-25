cmake_minimum_required(VERSION 3.20)

project(RubiksCubeOpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

add_library(glad STATIC
    external/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE
    ${CMAKE_SOURCE_DIR}/external
)

set(OPENGL_LIB "-framework OpenGL")

add_executable(rubiks_cube
    src/rubiks_cube.cpp
)

target_include_directories(rubiks_cube PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(rubiks_cube PRIVATE
    glad
    glfw
    glm
    ${OPENGL_LIB}
)