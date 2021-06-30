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

add_executable(glsb
    main.cpp
    shader.cpp
)
set_target_warnings(glsb)
target_compile_features(glsb
    PUBLIC
        cxx_std_20
)
target_link_libraries(glsb
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
