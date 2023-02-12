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

function(scriptor_add_flags target)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS OFF)
    target_compile_definitions(${target} PRIVATE SPDLOG_FMT_EXTERNAL SPDLOG_NO_THREAD_ID)
    if(MSVC)
        target_compile_definitions(${target} PRIVATE SCRIPTOR_WINDOWS _CRT_SECURE_NO_WARNINGS)
        target_compile_options(${target} PRIVATE /W4 /WX /bigobj /EHsc /wd4503 /wd4996 /wd4702 /wd4100 /wd4706)
        if(${MSVC_VERSION} GREATER_EQUAL 1929)
            if(scriptor_enable_asan)
                target_compile_options(${target} PRIVATE /fsanitize=address)
            endif()
        endif()
    else()
        if(APPLE)
            target_compile_definitions(${target} PRIVATE SCRIPTOR_APPLE)
        else()
            target_compile_definitions(${target} PRIVATE SCRIPTOR_LINUX)
        endif()
        target_compile_options(${target} PRIVATE -Wall -Wconversion -Wextra -Wpedantic -Werror)
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
        if(APPLE)
            target_compile_options(${target} PRIVATE -Wno-unused-lambda-capture)
        endif()
    endif()
endfunction()

find_package(Threads)

include(conan.cmake)
conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

find_package(asio REQUIRED)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

set(scriptor_core_source_files
    ${PROJECT_SOURCE_DIR}/src/aio.h
    ${PROJECT_SOURCE_DIR}/src/acceptor.h
    ${PROJECT_SOURCE_DIR}/src/acceptor.cpp
    ${PROJECT_SOURCE_DIR}/src/element.h
    ${PROJECT_SOURCE_DIR}/src/element.cpp
    ${PROJECT_SOURCE_DIR}/src/logger.h
    ${PROJECT_SOURCE_DIR}/src/logger.cpp
    ${PROJECT_SOURCE_DIR}/src/scriptor.h
    ${PROJECT_SOURCE_DIR}/src/scriptor.cpp
    ${PROJECT_SOURCE_DIR}/src/server.h
    ${PROJECT_SOURCE_DIR}/src/server.cpp
    ${PROJECT_SOURCE_DIR}/src/session.h
    ${PROJECT_SOURCE_DIR}/src/session.cpp
    ${PROJECT_SOURCE_DIR}/src/socket.h
    ${PROJECT_SOURCE_DIR}/src/socket.cpp
)

add_library(scriptor_core ${scriptor_core_source_files})
scriptor_add_flags(scriptor_core)
target_link_libraries(scriptor_core asio::asio nlohmann_json::nlohmann_json spdlog::spdlog)
if(UNIX AND NOT APPLE)
    target_link_libraries(scriptor_core systemd)
endif()

add_executable(scriptor src/main.cpp)
scriptor_add_flags(scriptor)
target_link_libraries(scriptor scriptor_core)

install(TARGETS scriptor RUNTIME DESTINATION bin)

set(scriptor_test_source_files
    ${PROJECT_SOURCE_DIR}/test/helper.h
    ${PROJECT_SOURCE_DIR}/test/helper.cpp
    ${PROJECT_SOURCE_DIR}/test/main.cpp
    ${PROJECT_SOURCE_DIR}/test/test_element.cpp
    ${PROJECT_SOURCE_DIR}/test/test_logger.cpp
    ${PROJECT_SOURCE_DIR}/test/test_processor.cpp
    ${PROJECT_SOURCE_DIR}/test/test_scriptor.cpp
)

if(scriptor_build_tests)
    enable_testing()
    add_executable(scriptor_test ${scriptor_test_source_files})
    scriptor_add_flags(scriptor_test)
    target_link_libraries(scriptor_test scriptor_core GTest::GTest)
    add_test(scriptor_test scriptor_test)
endif()

set(scriptor_source_files
    ${PROJECT_SOURCE_DIR}/src/scriptor.cpp
    ${scriptor_core_source_files}
    ${scriptor_test_source_files})

add_custom_target(
    scriptor_format
    COMMAND clang-format
    -style=file
    -i
    ${scriptor_source_files}
)
