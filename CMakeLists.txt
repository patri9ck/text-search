cmake_minimum_required(VERSION 3.16)
project(text_search)

set(CMAKE_CXX_STANDARD 20)

add_executable(text-search src/main.cpp
        src/candidate_v3/candidate_v3_text_search.cpp
        src/std/std_text_search.cpp
        src/hash_v1/hash_v1_text_search.cpp
        src/util.cpp)

add_executable(text-search-test
        # Misc
        src/test.cpp
        src/timer.cpp
        src/util.cpp

        # Sequential
        src/candidate_v1/candidate_v1_text_search.cpp
        src/candidate_v2/candidate_v2_text_search.cpp
        src/candidate_v3/candidate_v3_text_search.cpp
        src/candidate_v4/candidate_v4_text_search.cpp
        src/std/std_text_search.cpp
        src/hash_v1/hash_v1_text_search.cpp
        src/hash_v2/hash_v2_text_search.cpp

        # OpenMP
        src/candidate_openmp_v1/candidate_openmp_v1_text_search.cpp
        src/candidate_openmp_v2/candidate_openmp_v2_text_search.cpp
        src/hash_openmp_v1/hash_openmp_v1_text_search.cpp
        src/std_openmp/std_openmp_text_search.cpp

        # OpenMPI
        src/candidate_openmpi_v1/candidate_openmpi_v1_text_search.cpp

        # OpenCL
        src/candidate_opencl_v1/candidate_opencl_v1_text_search.cpp
        src/candidate_opencl_v2/candidate_opencl_v2_text_search.cpp
        src/candidate_opencl_v3/candidate_opencl_v3_text_search.cpp)

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

add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
