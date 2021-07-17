find_package(spdlog REQUIRED)

add_executable(glsb
    glsb.cpp
)
set_target_warnings(glsb)
target_compile_features(glsb
    PUBLIC
        cxx_std_20
)
target_link_libraries(glsb
    PUBLIC
        glsb::lib
        spdlog::spdlog
)

find_package(Vulkan REQUIRED)

add_executable(vksb
    vksb.cpp
)
set_target_warnings(vksb)
target_compile_features(vksb
    PUBLIC
        cxx_std_20
)
target_link_libraries(vksb
    PUBLIC
        glsb::lib
        spdlog::spdlog
        Vulkan::Vulkan
)
target_precompile_headers(vksb
    PUBLIC
        <vulkan/pch.h>
)
