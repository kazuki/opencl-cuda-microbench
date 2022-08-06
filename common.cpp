#include <limits>
#include <iostream>
#include "common.hpp"

void print_result(const char* name, const std::vector<double>& data)
{
    auto sum = 0.0;
    auto min = std::numeric_limits<double>::max();
    auto max = std::numeric_limits<double>::min();
    for (auto v : data) {
        sum += v;
        min = std::min(min, v);
        max = std::max(max, v);
    }
    auto mean = sum / data.size();
    auto var = 0.0;
    for (auto v : data) {
        var += (v - mean) * (v - mean);
    }
    var /= data.size();
    std::cout << name << ": " << mean << "us (var:" << var << ", min:" << min << ", max:" << max << ")" << std::endl;
}
