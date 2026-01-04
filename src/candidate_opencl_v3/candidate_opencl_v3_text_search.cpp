#include "candidate_opencl_v3_text_search.h"

#include <CL/opencl.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#ifdef BENCHMARK
Timer candidate_opencl_v3_timer = Timer(std::string("candidate_opencl_v3"));
#endif

// Helper to check for OpenCL errors
static void checkErr(cl_int err, const char *name) {
    if (err != CL_SUCCESS) {
        std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

static const char *kernel_source = R"raw(
__kernel void multi_search(
    __global const char* book,
    long book_len,
    __global const char* queries,
    __global const int* offsets,
    __global const int* lengths,
    __global int* out_char_indices, // Position im Text
    __global int* out_query_ids,    // Welches Wort wurde gefunden?
    __global int* global_counter,   // Ein einziger Zähler für alle
    int max_total_results)
{
    long char_idx = get_global_id(0);
    int query_idx = get_global_id(1);

    if (char_idx > (book_len - lengths[query_idx])) return;

    int q_start = offsets[query_idx];
    int q_len = lengths[query_idx];

    bool match = true;
    for (int i = 0; i < q_len; i++) {
        if (book[char_idx + i] != queries[q_start + i]) {
            match = false; break;
        }
    }

    if (match) {
        // Sicherer globaler Zähler
        int pos = atomic_inc(global_counter);
        if (pos < max_total_results) {
            out_char_indices[pos] = (int)char_idx;
            out_query_ids[pos] = query_idx;
        }
    }
}
)raw";

std::vector<std::vector<size_t>>
find_candidate_opencl_v3(const std::string &text, const std::vector<std::string> &queries) {
    cl_int err;
    int num_queries = (int)queries.size();
    long text_len = (long)text.size();

    int max_total_results = 20000000;

    // --- Standard Setup (Plattform, Device, Context, Queue) ---
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

    // --- Program & Kernel ---
    cl_program program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "multi_search", &err);

    // --- Daten vorbereiten ---
    std::string packed_queries = "";
    std::vector<int> h_offsets, h_lengths;
    for (const auto &q : queries) {
        h_offsets.push_back((int)packed_queries.size());
        h_lengths.push_back((int)q.size());
        packed_queries += q;
    }

    // --- Buffer erstellen ---
    cl_mem book_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, text_len, (void*)text.data(), &err);
    cl_mem queries_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, packed_queries.size(), (void*)packed_queries.data(), &err);
    cl_mem offsets_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_offsets.size()*4, h_offsets.data(), &err);
    cl_mem lengths_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, h_lengths.size()*4, h_lengths.data(), &err);

    // NEU: Die zwei Output-Listen und der globale Zähler
    cl_mem indices_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, max_total_results * sizeof(int), NULL, &err);
    cl_mem qids_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, max_total_results * sizeof(int), NULL, &err);

    int zero = 0;
    cl_mem counter_buf = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &zero, &err);

    // --- Kernel Argumente ---
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &book_buf);
    clSetKernelArg(kernel, 1, sizeof(long), &text_len);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &queries_buf);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &offsets_buf);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &lengths_buf);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), &indices_buf);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), &qids_buf);
    clSetKernelArg(kernel, 7, sizeof(cl_mem), &counter_buf);
    clSetKernelArg(kernel, 8, sizeof(int), &max_total_results);

    size_t local_size[2] = {256, 1};
    size_t global_size[2] = {((text_len + 255)/256)*256, (size_t)num_queries};
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);
    clFinish(queue);

    // --- Ergebnisse abholen ---
    int final_count = 0;
    clEnqueueReadBuffer(queue, counter_buf, CL_TRUE, 0, sizeof(int), &final_count, 0, NULL, NULL);

    int actual_read = std::min(final_count, max_total_results);
    std::vector<int> h_indices(actual_read);
    std::vector<int> h_qids(actual_read);

    clEnqueueReadBuffer(queue, indices_buf, CL_TRUE, 0, actual_read * sizeof(int), h_indices.data(), 0, NULL, NULL);
    clEnqueueReadBuffer(queue, qids_buf, CL_TRUE, 0, actual_read * sizeof(int), h_qids.data(), 0, NULL, NULL);

    // --- Ergebnisse sortieren (CPU) ---
    std::vector<std::vector<size_t>> final_indices(num_queries);
    for (int i = 0; i < actual_read; i++) {
        final_indices[h_qids[i]].push_back((size_t)h_indices[i]);
    }

    // WICHTIG: Die GPU liefert die Indizes unsortiert. std::find erwartet sie sortiert.
    for (int q = 0; q < num_queries; q++) {
        std::sort(final_indices[q].begin(), final_indices[q].end());
    }

    // --- Cleanup ---
    clReleaseMemObject(book_buf); clReleaseMemObject(queries_buf);
    clReleaseMemObject(offsets_buf); clReleaseMemObject(lengths_buf);
    clReleaseMemObject(indices_buf); clReleaseMemObject(qids_buf);
    clReleaseMemObject(counter_buf);
    clReleaseKernel(kernel); clReleaseProgram(program);
    clReleaseCommandQueue(queue); clReleaseContext(context);

    return final_indices;
}