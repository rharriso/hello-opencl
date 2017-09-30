run: hello_opencl
	./hello_opencl

hello_opencl: main.cpp Makefile
	$(CXX) main.cpp -std=c++1z -framework OpenCL -o hello_opencl