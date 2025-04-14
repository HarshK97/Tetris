cmake_minimum_required(VERSION 3.10)
project(tetris_opengl)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    include
)

add_executable(tetris_opengl main.cpp src/glad.c)
target_link_libraries(tetris_opengl ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES})
