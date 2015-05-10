cmake_minimum_required(VERSION 3.2)
project(gpgpu)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#
# gpgpu
#
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

pkg_search_module(EIGEN3 REQUIRED eigen3)
include_directories(${EIGEN3_INCLUDE_DIRS})

include_directories(include)

#
# Example
#
set(SOURCE_FILES main.cpp)
add_executable(gpgpu ${SOURCE_FILES})
target_link_libraries(gpgpu OpenImageIO boost_system ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES})