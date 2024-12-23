# logger

一个简单的日志库，支持日志级别、日志格式化、日志输出到文件、日志输出到控制台等功能。

A simple logger library, support log level, log format, log output to file, log output to console and so on.

## Lite

基于2.0.1, 更轻量化的版本，可用于嵌入式系统

## How to use

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DNEAPU_LOG_LEVEL=NEAPU_LOG_LEVEL_DEBUG)
else()
    add_definitions(-DNEAPU_LOG_LEVEL=NEAPU_LOG_LEVEL_INFO)
endif ()
add_subdirectory(logger)
```

```c++
#include "logger/logger.h"

// ...

NEAPU_LOGI("Hello, {}", "World");
NEAPU_LOGW("Hello, {}", "World");

NEAPU_LOGI_STREAM << "Hello, " << "World";

// ...
```