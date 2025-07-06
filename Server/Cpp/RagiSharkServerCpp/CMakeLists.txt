cmake_minimum_required(VERSION 4.0.3)

project(RagiSharkServerCpp VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINDOWS -D_WIN64)
    add_compile_options(/W4)
    set_property(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT "${CMAKE_PROJECT_NAME}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES Clang|GNU|AppleClang)
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-Wno-unused-function -Wno-unused-parameter -Wno-padded -Wno-sign-conversion -Wno-sign-compare)
endif()

# add_definitions(-DUNICODE -D_UNICODE)

include_directories(include)

set(RagiSharkServerCpp_Sources
    src/diagnostics/windows/process.cpp
    src/net/socket.cpp
    src/net/websocket.cpp
    src/util/util_windows.cpp
    src/main.cpp
)

add_executable(RagiSharkServerCpp ${RagiSharkServerCpp_Sources})

if(WIN32)
    target_link_libraries(RagiSharkServerCpp
        ws2_32
        crypt32
    )
endif()
