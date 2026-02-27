# logger

一个简单的日志库，支持日志级别、日志格式化、日志输出到文件、日志输出到控制台等功能。

A simple logger library, support log level, log format, log output to file, log output to console and so on.

## How to use

### CMake Configuration

```cmake
# logger requires C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(logger)
target_link_libraries(your_target PRIVATE logger)
```

### C++ Code

```c++
#include "logger/logger.h"
using namespace logger;

// ...

// Optional: console and file level
Logger::setPrintLevel(LogLevel::INFO);   // print <= INFO to console
Logger::setLogLevel(LogLevel::DEBUG);    // write <= DEBUG to file

// Optional: file output
Logger::setLogPath("./logs");

// Optional: file rotate (default 1MB)
Logger::setMaxFileSize(5 * 1024 * 1024); // 5MB

// Optional: enable/disable lock (default true)
Logger::setLockingEnabled(true);

// Basic logging (default channel: "Default")
LogInfo() << "service started";
LogWarn() << "memory high: " << 82 << "%";

// Channel logging (separate files per channel)
LogDebug("NET") << "connected to " << host;
LogError("DB").format("query failed, code={} sql={}", code, sql);

// Function trace (RAII)
void myFunction() {
    FunctionTracer trace(LogLevel::INFO, "CORE");
    // ...
} // auto emit [EXIT]

// ...
```

### Log Output Behavior

- Each message includes: time, channel, level, thread id, file name and line number.
- If `setLogPath()` is set, logs are written to `<logPath>/<channel>.log`.
- When file size exceeds `setMaxFileSize()`, old file is moved to `<logPath>/old/<channel>_<timestamp>.log`.
- First write of each file appends a header line with process id.

### Requirements

- **C++20**
- Supported platforms: Linux / macOS / Windows (Android links `log` automatically in CMake)