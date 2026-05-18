cmake_minimum_required(VERSION 3.20)
project(logger VERSION 4.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
