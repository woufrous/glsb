cmake_minimum_required(VERSION 3.0.0)
project(glsb VERSION 0.1.0)

include(CTest)
enable_testing()

find_package(OpenGL REQUIRED)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

#add_library(glsb_lib glsb.cpp)

add_executable(glsb
    src/main.cpp
    src/imgui/imgui_ogl3.cpp
    src/imgui/imgui_glfw.cpp
)
target_compile_options(glsb
    PUBLIC
        $<$<CXX_COMPILER_ID:GNU,Clang>:
            -Wall -Wextra -Wpedantic
        >
        $<$<CXX_COMPILER_ID:MSVC>:
            /W4
        >
)
target_compile_features(glsb
    PUBLIC
        cxx_std_20
)
target_link_libraries(glsb
    PUBLIC
        OpenGL::GL
        ${CONAN_LIBS}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
