cmake_minimum_required(VERSION 3.15)
project(HFT_Basics)

# FORCE C++17 OR C++20 SO <charconv> WORKS
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tells CMake to generate the compile_commands.json for VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------
# 1. FIND DEPENDENCIES
# ---------------------------------------------------------
# Silence the CMP0167 Boost Warning in newer CMake versions
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

# ---------------------------------------------------------
# 2. CREATE THE EXECUTABLE (Must happen BEFORE linking!)
# ---------------------------------------------------------
add_executable(HFT_Basics 
    main.cpp 
    simdjson.cpp 
    BinanceSession.cpp
    Parser.cpp
    Spinlock.cpp
)

target_include_directories(HFT_Basics PRIVATE ${Boost_INCLUDE_DIRS})

# ---------------------------------------------------------
# 3. LINK LIBRARIES TO EXECUTABLE
# ---------------------------------------------------------
target_link_libraries(HFT_Basics 
    PRIVATE 
    ${Boost_LIBRARIES} 
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# --- WINDOWS SPECIFIC LIBRARIES ---
if(WIN32)
    target_link_libraries(HFT_Basics PRIVATE ws2_32 wsock32)
endif()