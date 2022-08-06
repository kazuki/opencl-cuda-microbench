#include <sstream>
#include <vector>
#include <chrono>

#include "common.hpp"

__global__ void cuda_empty_kernel() {}

__global__ void cuda_nested_kernel(unsigned int ttl) {
    if (ttl > 0) {
        cuda_nested_kernel<<<1, 1>>>(ttl - 1);
    }
}

void bench_cuda_kernel_latency()
{
    size_t work_sizes[] = {1, 10, 100, 1000};
    cuda_empty_kernel<<<1, 1>>>();
    for(auto ws : work_sizes) {
        std::vector<double> times;
        for (auto i = 0; i < 1000; i ++) {
            auto s = std::chrono::high_resolution_clock::now();
            cuda_empty_kernel<<<ws, 1>>>();
            cudaDeviceSynchronize();
            auto e = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::micro>(e - s).count());
        }

        std::ostringstream name;
        name << "CUDA kernel launch latency(worksize=" << ws << ")";
        print_result(name.str().c_str(), times);
    }
}

void bench_cuda_nested_kernel_latency()
{
    unsigned int nest_depths[] = {1, 2, 4, 8, 16};
    cuda_nested_kernel<<<1, 1>>>(0);
    for(auto nest_depth : nest_depths) {
        std::vector<double> times;
        for (auto i = 0; i < 10; i ++) {
            auto s = std::chrono::high_resolution_clock::now();
            cuda_nested_kernel<<<1, 1>>>(nest_depth);
            cudaDeviceSynchronize();
            auto e = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::micro>(e - s).count());
        }

        std::ostringstream name;
        name << "CUDA nested kernel launch latency(depth=" << nest_depth << ")";
        print_result(name.str().c_str(), times);
    }
}
