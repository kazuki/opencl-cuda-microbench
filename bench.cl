__kernel void empty_kernel() {}

__kernel void nested_kernel(unsigned int ttl) {
    if (ttl > 0) {
        enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange_1D(1), ^{
            nested_kernel(ttl - 1);
        });
    }
}
