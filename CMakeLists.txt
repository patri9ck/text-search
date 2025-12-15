cmake_minimum_required(VERSION 3.16)
project(text_search)

set(CMAKE_CXX_STANDARD 20)

add_executable(text-search src/main.cpp
        src/sequential/sequential_text_search.cpp
        src/std/std_text_search.cpp
        src/hash/hash_text_search.cpp
        src/util.cpp)
add_executable(text-search-benchmark
        src/benchmark.cpp
        src/timer.cpp
        src/util.cpp
        src/sequential/sequential_text_search.cpp src/sequential/sequential_text_search_benchmark.cpp
        src/std/std_text_search.cpp src/std/std_text_search_benchmark.cpp
        src/hash/hash_text_search.cpp src/hash/hash_text_search_benchmark.cpp)

target_include_directories(text-search PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(text-search-benchmark PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(text-search-benchmark PRIVATE BENCHMARK)
