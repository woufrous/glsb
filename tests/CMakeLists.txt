find_package(Catch2 REQUIRED)

add_executable(unittests
    main.cpp
    tests_dummy.cpp
)
set_target_warnings(unittests)
target_link_libraries(unittests
    PRIVATE
        Catch2::Catch2
)

include(CTest)
include(Catch)
catch_discover_tests(unittests)
