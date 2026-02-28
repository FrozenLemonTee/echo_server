cmake_minimum_required(VERSION 3.31)
project(echo_server)

set(CMAKE_CXX_STANDARD 23)

set(BUILD_TESTING OFF CACHE BOOL "Disable tests in the fetched project")

include(FetchContent)

FetchContent_Declare(
        original
        GIT_REPOSITORY git@github.com:FrozenLemonTee/original.git
        GIT_TAG dev
)

FetchContent_MakeAvailable(original)

add_executable(echo_server main.cpp)


target_link_libraries(echo_server PRIVATE original)
if(WIN32)
    target_link_libraries(echo_server PRIVATE ws2_32)
endif()
