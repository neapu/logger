cmake_minimum_required(VERSION 3.20)
project(logger VERSION 4.0.1)


include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.2.0
)
FetchContent_MakeAvailable(fmt)


if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    endif()
endif()
add_library(${PROJECT_NAME} STATIC logger.h logger.cpp)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /Zc:__cplusplus)
endif()
if (LINUX OR APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()
if (ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE log)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)