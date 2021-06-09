cmake_minimum_required(VERSION 3.0.0)

include(cmake/build_type.cmake)
include(cmake/init_conan.cmake)
include(cmake/compiler_warnings.cmake)

select_build_type()

project(glsb VERSION 0.1.0)

init_conan()

conan_cmake_configure(
    REQUIRES
        glew/2.2.0
        glfw/3.3.4
        glm/0.9.9.8
        imgui/1.81
        spdlog/1.8.5
        stb/20200203
        tinyobjloader/1.0.6
    GENERATORS cmake_find_package
)

conan_cmake_autodetect(settings)

conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    REMOTE conan-center
    SETTINGS ${settings}
)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    add_compile_options(-fcolor-diagnostics)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
endif()

add_subdirectory(src)
