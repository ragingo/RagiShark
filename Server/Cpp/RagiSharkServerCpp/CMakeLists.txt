cmake_minimum_required(VERSION 3.16)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(RagiSharkServerCpp)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINDOWS -D_WIN64)
    add_compile_options(/W4)
    set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${CMAKE_PROJECT_NAME}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES Clang|GNU|AppleClang)
    # add_definitions(-DUNICODE -D_UNICODE)
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-Wno-unused-function -Wno-unused-parameter -Wno-padded -Wno-sign-conversion -Wno-sign-compare)
endif()

include_directories(include)

set(RagiSharkServerCpp_Sources
    src/diagnostics/macos/process.cpp
    src/diagnostics/windows/process.cpp
    src/net/macos/socket.cpp
    src/net/windows/socket.cpp
    src/net/websocket.cpp
    src/util/macos/util.cpp
    src/util/windows/util.cpp
    src/main.cpp
)

add_executable(RagiSharkServerCpp ${RagiSharkServerCpp_Sources})
