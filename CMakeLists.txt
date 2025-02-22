cmake_minimum_required(VERSION 3.20)
project(sxssion)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sxssion main.cpp)

target_compile_options(sxssion PRIVATE
    "-std=c++23"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wshadow"
    "-Wconversion"
    "-Wunused"
    "-O3"
    "-march=native"
)

add_subdirectory(json)
target_link_libraries(sxssion PRIVATE nlohmann_json::nlohmann_json)
