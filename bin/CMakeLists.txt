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
