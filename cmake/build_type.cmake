macro(select_build_type)
    if(NOT DEFINED CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type." FORCE)
    endif()
    set_property(
        CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS
            "Debug"
            "Release"
            "RelWithDebInfo"
            "MinSizeRel"
    )
endmacro()
