find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(spdlog REQUIRED)
find_package(stb REQUIRED)
find_package(tinyobjloader REQUIRED)

add_library(glsb_imgui_bindings
    imgui/imgui_ogl3.cpp
    imgui/imgui_glfw.cpp
)
target_link_libraries(glsb_imgui_bindings
    PRIVATE
        GLEW::GLEW
        glfw::glfw
        imgui::imgui
)

add_library(glsb_lib
    shader.cpp
)
target_include_directories(glsb_lib
    PUBLIC
        .
)
add_library(glsb::lib ALIAS glsb_lib)
set_target_warnings(glsb_lib)
target_compile_features(glsb_lib
    PUBLIC
        cxx_std_20
)
target_link_libraries(glsb_lib
    PUBLIC
        glsb_imgui_bindings
        GLEW::GLEW
        glfw::glfw
        glm::glm
        imgui::imgui
        spdlog::spdlog
        stb::stb
        tinyobjloader::tinyobjloader
)
