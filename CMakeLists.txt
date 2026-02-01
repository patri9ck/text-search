cmake_minimum_required(VERSION 3.16)
project(text_search)

set(CMAKE_CXX_STANDARD 17)

add_executable(text-search
        # Misc
        src/main.cpp
        src/timer.cpp
        src/util.cpp

        # Sequential
        src/std/std_text_search.cpp

        # OpenMP
        src/std_openmp/std_openmp_text_search.cpp

        # MPI
        src/candidate_mpi/candidate_mpi_text_search.cpp

        # OpenCL
        src/candidate_opencl_v2/candidate_opencl_v2_text_search.cpp
        src/candidate_opencl_v3/candidate_opencl_v3_text_search.cpp)

add_executable(text-search-test
        # Misc
        src/test.cpp
        src/timer.cpp
        src/util.cpp

        # Sequential
        src/std/std_text_search.cpp
        src/candidate_v1/candidate_v1_text_search.cpp
        src/candidate_v2/candidate_v2_text_search.cpp
        src/candidate_v3/candidate_v3_text_search.cpp
        src/candidate_v4/candidate_v4_text_search.cpp
        src/hash_v1/hash_v1_text_search.cpp
        src/hash_v2/hash_v2_text_search.cpp

        # OpenMP
        src/std_openmp/std_openmp_text_search.cpp
        src/candidate_openmp_v1/candidate_openmp_v1_text_search.cpp
        src/candidate_openmp_v2/candidate_openmp_v2_text_search.cpp
        src/hash_openmp/hash_openmp_text_search.cpp

        # MPI
        src/candidate_mpi/candidate_mpi_text_search.cpp

        # OpenCL
        src/candidate_opencl_v1/candidate_opencl_v1_text_search.cpp
        src/candidate_opencl_v2/candidate_opencl_v2_text_search.cpp
        src/candidate_opencl_v3/candidate_opencl_v3_text_search.cpp)

add_subdirectory(third_party/cxxopts)

find_package(OpenCL REQUIRED)
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)

target_link_libraries(text-search PRIVATE
        cxxopts::cxxopts
        OpenCL::OpenCL
        OpenMP::OpenMP_CXX
        MPI::MPI_CXX
)

target_link_libraries(text-search-test PRIVATE
        cxxopts::cxxopts
        OpenCL::OpenCL
        OpenMP::OpenMP_CXX
        MPI::MPI_CXX
)

target_compile_definitions(text-search-test PRIVATE BENCHMARK)

add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
