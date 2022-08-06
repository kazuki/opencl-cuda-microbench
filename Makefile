all: bench_all bench_ocl
clean:
	rm -f bench_all bench_ocl opencl.inc

bench_all: *.cpp *.hpp *.cl *.cu opencl.inc
	nvcc -lOpenCL -rdc=true -O2 -o $@ *.cpp *.cu

bench_ocl: *.cpp *.hpp *.cl opencl.inc
	g++ -lOpenCL -O2 -o $@ *.cpp

opencl.inc: bench.cl
	cat $< | python3 .bin2c.py opencl_program_code > $@
