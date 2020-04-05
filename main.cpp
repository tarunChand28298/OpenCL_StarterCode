#pragma comment(lib, "OpenCL.lib")
#include <CL/cl.h>

//Use this for checking errors on host side:
const char* getErrorString(cl_int error)
{
	switch (error) {
		// run-time and JIT compiler errors
		case  0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
		case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case -15: return "CL_COMPILE_PROGRAM_FAILURE";
		case -16: return "CL_LINKER_NOT_AVAILABLE";
		case -17: return "CL_LINK_PROGRAM_FAILURE";
		case -18: return "CL_DEVICE_PARTITION_FAILED";
		case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		// compile-time errors
		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case -64: return "CL_INVALID_PROPERTY";
		case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case -66: return "CL_INVALID_COMPILER_OPTIONS";
		case -67: return "CL_INVALID_LINKER_OPTIONS";
		case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

		default: return "Unknown OpenCL error";
	}
}
//if (result != CL_SUCCESS) std::cerr << getErrorString(result) << std::endl;

//Use this for checking errors in the shader:
//if (result != CL_SUCCESS) std::cerr << getErrorString(result) << std::endl;
//if (result == CL_BUILD_PROGRAM_FAILURE) {
//	// Determine the size of the log
//	size_t log_size;
//	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
//
//	// Allocate memory for the log
//	char* log = (char*)malloc(log_size);
//
//	// Get the log
//	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
//
//	// Print the log
//	printf("%s\n", log);
//}

#define NumberOfFloats 200

float inputData[NumberOfFloats];
float outputData[NumberOfFloats];

int main()
{
	for (int i = 0; i < NumberOfFloats; i++)
	{
		inputData[i] = float(i + 1);
	}

	//===============================================================================================

	cl_device_id device;
	cl_context context;
	cl_command_queue commandQueue;

	cl_program program;
	cl_kernel kernel;

	cl_mem inputBuffer;
	cl_mem outputBuffer;

	//===============================================================================================

	//Create device:
	{
		cl_platform_id* platformIDs;
		cl_uint platforms;
		clGetPlatformIDs(1, nullptr, &platforms);
		platformIDs = new cl_platform_id[platforms];
		clGetPlatformIDs(platforms, platformIDs, nullptr);

		for (int i = 0; i < platforms; i++)
		{
			char pName[100];
			size_t read;
			clGetPlatformInfo(platformIDs[i], CL_PLATFORM_NAME, sizeof(char) * 100, pName, &read);
		}

		clGetDeviceIDs(platformIDs[0], CL_DEVICE_TYPE_GPU, 1, &device, NULL);

		delete[] platformIDs;
	}

	//Create context and command queue:
	context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
	commandQueue = clCreateCommandQueue(context, device, (cl_command_queue_properties)0, NULL);

	//Compile and create kernel:
	{
		const char* programSource = "kernel void GPUMain(global float* input, global float* output)\
								 {\
								 	int id = get_global_id(0);\
								 	output[id] = input[id] * 2.0f;\
								 }";

		program = clCreateProgramWithSource(context, 1, &programSource, NULL, NULL);
		auto result = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

		kernel = clCreateKernel(program, "GPUMain", NULL);
	}

	//Setup inputs and outputs:
	{
		inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * NumberOfFloats, NULL, NULL);
		outputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * NumberOfFloats, NULL, NULL);
		clEnqueueWriteBuffer(commandQueue, inputBuffer, CL_FALSE, 0, sizeof(cl_float) * NumberOfFloats, (void*)inputData, 0, NULL, NULL);
	}

	//Execute the program:
	{
		clSetKernelArg(kernel, 0, sizeof(inputBuffer), &inputBuffer);
		clSetKernelArg(kernel, 1, sizeof(outputBuffer), &outputBuffer);
		size_t globalDimensions[] = { NumberOfFloats, 0, 0 };
		clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalDimensions, NULL, 0, NULL, NULL);
	}

	//Read back the result:
	{
		clEnqueueReadBuffer(commandQueue, outputBuffer, CL_FALSE, 0, sizeof(cl_float) * NumberOfFloats, (void*)outputData, 0, NULL, NULL);
	}

	clFinish(commandQueue);
	//===============================================================================================

	return 0;
}
