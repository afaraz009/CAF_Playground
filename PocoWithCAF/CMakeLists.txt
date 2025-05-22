cmake_minimum_required(VERSION 3.12)
project(CAF_HelloWorld CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Include Directories ----
include_directories(
    D:/CODE/actor-framework/libcaf_net
    D:/CODE/actor-framework/build/libcaf_net
    D:/CODE/actor-framework/libcaf_core
    D:/CODE/actor-framework/build/libcaf_core
    C:/Users/ahmed/miniconda3/Library/include
)

find_package(Poco REQUIRED COMPONENTS Foundation Net NetSSL Util Crypto JSON)


# ---- Add your source file ----
add_executable(hello_world Source.cpp PocoOpenAIClient.cpp)  # Change Source.cpp if your file is named differently

# ---- Library Directories ----
link_directories(
    D:/CODE/actor-framework/build/libcaf_net/Debug
    D:/CODE/actor-framework/build/libcaf_core/Debug
    C:/Users/ahmed/miniconda3/Library/lib
)

# ---- Link Libraries ----
target_link_libraries(hello_world
    D:/CODE/actor-framework/build/libcaf_net/Debug/caf_net.lib
    D:/CODE/actor-framework/build/libcaf_core/Debug/caf_core.lib
    ws2_32.lib
    iphlpapi.lib
    C:/Users/ahmed/miniconda3/Library/lib/libssl.lib
    C:/Users/ahmed/miniconda3/Library/lib/libcrypto.lib
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    comdlg32.lib
    advapi32.lib
    Poco::Foundation
    Poco::Net
    Poco::JSON
    Poco::Util
    Poco::Crypto
    Poco::NetSSL
)

# ---- Compile Definitions ----
target_compile_definitions(hello_world PRIVATE
    WIN32
    _WINDOWS
    NDEBUG
    _CRT_SECURE_NO_WARNINGS
)

# ---- Warning level, runtime options, disable specific warnings ----
if (MSVC)
    # Equivalent to <WarningLevel>Level3</WarningLevel>
    target_compile_options(hello_world PRIVATE /W3)
    # Disable 4275, 4251 (DLL-interface warnings)
    target_compile_options(hello_world PRIVATE /wd4275 /wd4251)
    # Set runtime library to MultiThreadedDebugDLL (/MDd)
    set_property(TARGET hello_world PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    # Enable fast checks
    target_compile_options(hello_world PRIVATE /RTC1)
    # Disable external warnings (best effort in CMake)
    target_compile_options(hello_world PRIVATE /external:W0)
endif()

# ---- Set Subsystem to Console ----
set_target_properties(hello_world PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:CONSOLE"
)
