cmake_minimum_required(VERSION 3.16)
project(flappybird LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Executable
add_executable(flappybird main.cpp)

# Include SDL2 headers
target_include_directories(flappybird PRIVATE ${SDL2_INCLUDE_DIRS})

# Link libraries — ALL keyword-style
target_link_libraries(flappybird
    PRIVATE
    mingw32
    SDL2main
    SDL2
)
