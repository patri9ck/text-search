cmake_minimum_required(VERSION 3.16)
project(text_search)

set(CMAKE_CXX_STANDARD 20)

add_executable(text-search src/main.cpp
        src/candidate_v3/candidate_v3_text_search.cpp
        src/std/std_text_search.cpp
        src/hash/hash_text_search.cpp
        src/util.cpp)
add_executable(text-search-test
        src/test.cpp
        src/timer.cpp
        src/util.cpp
        src/candidate_v1/candidate_v1_text_search.cpp src/candidate_v1/candidate_v1_text_search_benchmark.cpp
        src/candidate_v2/candidate_v2_text_search.cpp src/candidate_v2/candidate_v2_text_search_benchmark.cpp
        src/candidate_v3/candidate_v3_text_search.cpp src/candidate_v3/candidate_v3_text_search_benchmark.cpp
        src/std/std_text_search.cpp src/std/std_text_search_benchmark.cpp
        src/hash/hash_text_search.cpp src/hash/hash_text_search_benchmark.cpp)

find_package(OpenCL REQUIRED)
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)

target_include_directories(text-search PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(text-search-test PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(text-search OpenCL::OpenCL)
target_link_libraries(text-search OpenMP::OpenMP_CXX)
target_link_libraries(text-search MPI::MPI_CXX)
target_link_libraries(text-search-test OpenCL::OpenCL)
target_link_libraries(text-search-test OpenMP::OpenMP_CXX)
target_link_libraries(text-search-test MPI::MPI_CXX)

target_compile_definitions(text-search-test PRIVATE BENCHMARK)
