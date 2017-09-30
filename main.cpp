#include <iostream>
#include "cl.hpp"

#define MAX_SOURCE_SIZE (0x100000)

int main()
{
  //get all platforms (drivers)
  std::vector<cl::Platform> all_platforms;
  cl::Platform::get(&all_platforms);

  if (all_platforms.empty())
  {
    std::cout << "No platforms found. Check OpenCL installation!\n";
    exit(1);
  }

  auto default_platform = all_platforms[0];
  std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << '\n';

  // get the default device for this platform
  std::vector<cl::Device> all_devices;
  default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
  if (all_devices.empty())
  {
    std::cout << "No devices found. Check OpenCL installation!\n";
    exit(1);
  }

  auto default_device = all_devices[0];
  for (auto device : all_devices)
  {
    std::cout << "Available device: " << device.getInfo<CL_DEVICE_NAME>() << '\n';
  }
  std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << '\n';

  // create the context and sources
  auto context = cl::Context({default_device});
  auto sources = cl::Program::Sources();

  std::string kernel_code = R"(
    void kernel simple_add(global const int* A, global const int* B, global int* C) {
      int id = get_global_id(0);
      C[id] = A[id] + B[id];
    }
  )";
  sources.push_back({kernel_code.c_str(), kernel_code.length()});

  // create program from context & sources
  auto program = cl::Program(context, sources);
  if (program.build({default_device}) != CL_SUCCESS)
  {
    std::cout << "Error Building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << '\n';
  }

  // create buffers on the devices
  auto buffer_size = sizeof(int) * 10;
  auto buffer_A = cl::Buffer(context, CL_MEM_READ_WRITE, buffer_size);
  auto buffer_B = cl::Buffer(context, CL_MEM_READ_WRITE, buffer_size);
  auto buffer_C = cl::Buffer(context, CL_MEM_READ_WRITE, buffer_size);

  int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
  int C[10];

  // create a queue with to push commands onto
  auto command_queue = cl::CommandQueue(context, default_device);
  // attach read and write byte arrays
  command_queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, buffer_size, A);
  command_queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, buffer_size, B);

  // create kernal and add args
  auto simple_add = cl::Kernel(program, "simple_add");
  simple_add.setArg(0, buffer_A);
  simple_add.setArg(1, buffer_B);
  simple_add.setArg(2, buffer_C);
  command_queue.enqueueNDRangeKernel(simple_add, cl::NullRange, cl::NDRange(10), cl::NullRange);
  command_queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, buffer_size, C);

  std::cout << "Ainput array [ ";
  for (auto cell : A)
  {
    std::cout << cell << ' ';
  }
  std::cout << "] \n";

  std::cout << "Binput array [ ";
  for (auto cell : B)
  {
    std::cout << cell << ' ';
  }
  std::cout << "] \n";

  std::cout << "output array [ ";
  for (auto cell : C)
  {
    std::cout << cell << ' ';
  }
  std::cout << "] \n";
}
