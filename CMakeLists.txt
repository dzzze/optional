cmake_minimum_required(VERSION 3.13)

project(dze_optional)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(thirdparty/dze_type_traits)

add_library(dze_optional INTERFACE)
target_include_directories(dze_optional INTERFACE include)
target_link_libraries(dze_optional INTERFACE dze::type_traits)
add_library(dze::optional ALIAS dze_optional)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(compiler_options)

    option(${PROJECT_NAME}_static_analyzer "Enable Clang-Tidy" ON)

    if (${PROJECT_NAME}_static_analyzer)
        include(clang-tidy)
    endif ()

    option(${PROJECT_NAME}_unit_tests "Enable unit tests" ON)

    if (${PROJECT_NAME}_unit_tests)
        add_custom_target(${PROJECT_NAME}-run-custom-tests ALL)
        add_subdirectory(test)
    endif ()
endif ()
