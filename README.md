# logger

一个简单的日志库，支持日志级别、日志格式化、日志输出到文件、日志输出到控制台等功能。

A simple logger library, support log level, log format, log output to file, log output to console and so on.

## How to use

### CMake Configuration

```cmake
# Set log level (Optional, default is DEBUG)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DNEAPU_LOG_LEVEL=NEAPU_LOG_LEVEL_DEBUG)
else()
    add_definitions(-DNEAPU_LOG_LEVEL=NEAPU_LOG_LEVEL_INFO)
endif ()

# Enable fmt library support (Optional, requires fmt library)
# set(ENABLE_FMT_LIB ON)

# Disable short macros (LOGI, LOGE, etc.) if they conflict with other libraries
# add_definitions(-DNEAPU_LOG_NO_SHORT_MACROS)

add_subdirectory(logger)
target_link_libraries(your_target PRIVATE logger)
```

### C++ Code

```c++
#include "logger/logger.h"

// ...

// Use short macros (if not disabled)
LOGI("Hello, {}", "World"); // Requires C++20 or ENABLE_FMT_LIB
LOGW("Warning message");

// Use stream style
LOGI_STREAM << "Hello, " << "World";

// Use full name macros (always available)
NEAPU_LOGE("Error: {}", 404);
NEAPU_LOGD_STREAM << "Debug info";

// Function tracing
void myFunction() {
    FUNC_TRACE; // Logs entry and exit of the function
    // ...
}

// ...
```

### Requirements

-   **C++20**: For `std::format` and `std::source_location` support by default.
-   **C++11/14/17**: Can be used with `ENABLE_FMT_LIB` (requires `fmt` library) for formatting support.
    -   Without `fmt` and < C++20: Formatting macros (`LOGI`, `NEAPU_LOGI` etc.) will degrade to `stringstream` (no format string support) or become no-ops depending on configuration, but compilation will succeed. Stream macros (`LOGI_STREAM`) work normally.