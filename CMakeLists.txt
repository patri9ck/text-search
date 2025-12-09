cmake_minimum_required(VERSION 3.16)
project(text_search)

set(CMAKE_CXX_STANDARD 20)

add_executable(text-search src/main.cpp src/sequential_text_search.cpp src/sequential_text_search_std.cpp src/util.cpp)

target_include_directories(text-search PRIVATE ${PROJECT_SOURCE_DIR}/include)
