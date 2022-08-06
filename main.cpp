#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <CL/cl.h>
#include "opencl.inc"

using HiResClock = std::chrono::high_resolution_clock;

void print_result(const char*, const std::vector<double>&);
void bench_kernel_latency(cl_context, cl_command_queue, cl_program);
void bench_nested_kernel_latency(cl_context, cl_command_queue, cl_program);

#ifdef __NVCC__
void bench_cuda_kernel_latency();
void bench_cuda_nested_kernel_latency();
#endif

int main() {
  cl_int ret;
  cl_uint ret_num;
  cl_platform_id platform_id;
  cl_device_id device_id;
  cl_context context;
  clGetPlatformIDs(1, &platform_id, &ret_num);
  if (ret_num > 0) {
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num);
    if (ret_num > 0)
      context = clCreateContext(NULL, 1, &device_id, NULL, NULL, NULL);
  }
  if (device_id == NULL) return -1;
  cl_command_queue cmd_queue_ =
      clCreateCommandQueueWithProperties(context, device_id, NULL, &ret);
  if (ret != CL_SUCCESS) return -1;

  const char* tmp = opencl_program_code;
  cl_program program = clCreateProgramWithSource(context, 1, &tmp, &opencl_program_code_size, &ret);
  if (ret != CL_SUCCESS) return -1;

  ret = clBuildProgram(program, 1, &device_id, "-cl-std=CL2.0", NULL, NULL);
  if (ret != CL_SUCCESS) {
    std::cerr << "[FAILED] clBuildProgram Failed" << std::endl;
    size_t log_size;
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    if (log_size > 0) {
        auto buf = new char[log_size];
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, buf, &log_size);
        std::cout << buf << std::endl;
        delete[] buf;
    }
    clReleaseProgram(program);
    return -1;
  }

  bench_kernel_latency(context, cmd_queue_, program);
  bench_nested_kernel_latency(context, cmd_queue_, program);
#ifdef __NVCC__
  bench_cuda_nested_kernel_latency();
  bench_cuda_kernel_latency();
#endif
}

void bench_kernel_latency(cl_context context, cl_command_queue queue, cl_program program)
{
    cl_kernel kernel = clCreateKernel(program, "empty_kernel", NULL);
    size_t work_sizes[] = {1, 10, 100, 1000};
    for(auto ws : work_sizes) {
        size_t global_work_size[] = {ws};
        std::vector<double> times;
        for (auto i = -1; i < 1000; i ++) {
            auto s = HiResClock::now();
            cl_int ret = clEnqueueNDRangeKernel(
                queue, kernel, 1, NULL, static_cast<size_t*>(global_work_size), NULL, 0, NULL, NULL);
            clFinish(queue);
            auto e = HiResClock::now();
            if (i >= 0)
                times.push_back(std::chrono::duration<double, std::micro>(e - s).count());
        }

        std::ostringstream name;
        name << "kernel launch latency(worksize=" << ws << ")";
        print_result(name.str().c_str(), times);
    }
}

void bench_nested_kernel_latency(cl_context context, cl_command_queue queue, cl_program program)
{
    cl_kernel kernel = clCreateKernel(program, "nested_kernel", NULL);
    size_t global_work_size[] = {1};
    cl_uint nest_depths[] = {1, 2, 4, 8, 16};
    for(auto nest_depth : nest_depths) {
        clSetKernelArg(kernel, 0, sizeof(cl_uint), &nest_depth);
        std::vector<double> times;
        for (auto i = -1; i < 10; i ++) {
            auto s = HiResClock::now();
            cl_int ret = clEnqueueNDRangeKernel(
                queue, kernel, 1, NULL, static_cast<size_t*>(global_work_size), NULL, 0, NULL, NULL);
            clFinish(queue);
            auto e = HiResClock::now();
            if (i >= 0)
                times.push_back(std::chrono::duration<double, std::micro>(e - s).count());
        }

        std::ostringstream name;
        name << "nested kernel launch latency(depth=" << nest_depth << ")";
        print_result(name.str().c_str(), times);
    }
}
