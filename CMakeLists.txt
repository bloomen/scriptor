cmake_minimum_required(VERSION 3.0)
project(scriptor)

option(scriptor_build_tests "Build scriptor tests." OFF)
option(scriptor_enable_asan "Build scriptor with address sanitizer." OFF)
option(scriptor_enable_tsan "Build scriptor with thread sanitizer." OFF)
option(scriptor_enable_coverage "Build scriptor with coverage reporting." OFF)

if(scriptor_enable_asan AND scriptor_enable_tsan)
    message(FATAL_ERROR "scriptor_enable_asan and scriptor_enable_tsan cannot both be ON")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/cmake/${CMAKE_BUILD_TYPE}.cmake")
    include("${PROJECT_SOURCE_DIR}/cmake/${CMAKE_BUILD_TYPE}.cmake")
endif()

function(scriptor_add_flags target)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS OFF)
    target_compile_definitions(${target} PRIVATE SPDLOG_FMT_EXTERNAL)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /bigobj /EHsc /wd4503 /wd4996 /wd4702 /wd4100 /wd4706)
        if(${MSVC_VERSION} GREATER_EQUAL 1929)
            if(scriptor_enable_asan)
                target_compile_options(${target} PRIVATE /fsanitize=address)
            endif()
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wconversion -Wextra -Wpedantic)
        target_link_libraries(${target} ${CMAKE_THREAD_LIBS_INIT})
        if(CMAKE_COMPILER_IS_GNUCC)
            target_compile_options(${target} PRIVATE -pthread)
        endif()
        if(scriptor_enable_asan)
            if(APPLE)
                target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
                set_target_properties(${target} PROPERTIES LINK_FLAGS "-fsanitize=address,undefined")
            else()
                target_compile_options(${target} PRIVATE -fsanitize=address,leak,undefined)
                set_target_properties(${target} PROPERTIES LINK_FLAGS "-fsanitize=address,leak,undefined")
            endif()
        endif()
        if(scriptor_enable_tsan)
            target_compile_options(${target} PRIVATE -fsanitize=thread)
            set_target_properties(${target} PROPERTIES LINK_FLAGS "-fsanitize=thread")
        endif()
        if(scriptor_enable_coverage)
            target_compile_options(${target} PRIVATE --coverage)
            set_target_properties(${target} PROPERTIES LINK_FLAGS "--coverage")
        endif()
    endif()
endfunction()

find_package(Threads)

set(scriptor_core_source_files
    ${PROJECT_SOURCE_DIR}/src/aio.h
    ${PROJECT_SOURCE_DIR}/src/element.h
    ${PROJECT_SOURCE_DIR}/src/element.cpp
    ${PROJECT_SOURCE_DIR}/src/server.h
    ${PROJECT_SOURCE_DIR}/src/server.cpp
    ${PROJECT_SOURCE_DIR}/src/session.h
    ${PROJECT_SOURCE_DIR}/src/session.cpp
)

add_library(scriptor_core ${scriptor_core_source_files})
scriptor_add_flags(scriptor_core)
target_link_libraries(scriptor_core systemd spdlog$<$<CONFIG:Debug>:d> fmt$<$<CONFIG:Debug>:d>)

add_executable(scriptor src/scriptor.cpp)
scriptor_add_flags(scriptor)
target_link_libraries(scriptor scriptor_core)

install(TARGETS scriptor RUNTIME DESTINATION bin)

if(scriptor_build_tests)
    enable_testing()
    add_executable(scriptor_test test/test.cpp)
    scriptor_add_flags(scriptor_test)
    target_include_directories(scriptor_test PRIVATE src)
    target_link_libraries(scriptor_test scriptor_core gtest$<$<CONFIG:Debug>:d>)
    add_test(scriptor_test scriptor_test)
endif()

set(scriptor_source_files
    ${PROJECT_SOURCE_DIR}/src/scriptor.cpp
    ${scriptor_core_source_files})

add_custom_target(
    scriptor_format
    COMMAND clang-format
    -style=file
    -i
    ${scriptor_source_files}
)
