find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(spdlog REQUIRED)
find_package(stb REQUIRED)
find_package(tinyobjloader REQUIRED)

add_executable(glsb
    main.cpp
    shader.cpp
    imgui/imgui_ogl3.cpp
    imgui/imgui_glfw.cpp
)
set_target_warnings(glsb)
target_compile_features(glsb
    PUBLIC
        cxx_std_20
)
target_link_libraries(glsb
    PUBLIC
        GLEW::GLEW
        glfw::glfw
        glm::glm
        imgui::imgui
        spdlog::spdlog
        stb::stb
        tinyobjloader::tinyobjloader
)