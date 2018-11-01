#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include <CL/cl.h>

//OpenCL kernel to perform an element-wise addition
const char* programSource =
"--kernel \n"
"void vecadd(__global int *A          \n"
"            __global int *B          \n"
"            __global int *C)          \n"
"{                                             \n"
"                                              \n"
"  //Get the work-item's unique ID             \n"
"  int idx = get_global_id(0);                 \n"
"                                              \n"
"  //Add the corresponding locations of        \n"
"  //'A' and 'B', and store the result in 'C'  \n"
"  C[idx] = A[idx] + B[idx];                   \n"
"}                                             \n"
;

int main(void) 
{
  const int elements = 2048;
  size_t datasize = sizeof( int ) *elements;
  
  //Allocate space for input/output host data
  int *A = (int*)malloc( datasize ); //input array
  int *B = (int*)malloc( datasize ); //input array
  int *C = (int*)malloc( datasize ); //output array
  int i;
  for ( i = 0; i < elements; i++ )
  {
    A[i] = i;
    B[i] = i;
  }

	cl_int status;
	cl_platform_id* platform;
	cl_int NumOfPlatforms;

	status = clGetPlatformIDs(0, NULL, &NumOfPlatforms);
	platform = (cl_platform_id*)malloc(sizeof(cl_platform_id) * NumOfPlatforms);
	status = clGetPlatformIDs(NumOfPlatforms, platform, NULL);
	assert(status == CL_SUCCESS);
	//printf("In the system there are %d platforms detected.\n", NumOfPlatforms);

	cl_device_id* device;
	cl_int NumberOfDevices;
	status = clGetDeviceIDs(platform[1], CL_DEVICE_TYPE_ALL, 0, NULL, &NumberOfDevices);
	device = (cl_device_id*)malloc(sizeof(cl_device_id)*NumberOfDevices);
	status = clGetDeviceIDs(platform[1], CL_DEVICE_TYPE_ALL, NumberOfDevices, device, NULL);

	for (int j = 0; j < NumberOfDevices; j++)
	{
		cl_char vendor_name[1024] = { 0 };
		cl_char device_name[1024] = { 0 };
		size_t returned_size = 0;

		status = clGetDeviceInfo(device[j], CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
		status = clGetDeviceInfo(device[j], CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
		printf("Available device %s, %s\n", vendor_name, device_name);
	}

  cl_context context = clCreateContext( NULL, 1, device, NULL, NULL, &status );
  cl_command_queue cmdQueue = clCreateCommandQueue(context, device[0], 0, &status);

  cl_mem bufA = clCreateBuffer( context, CL_MEM_READ_ONLY, datasize, NULL, &status );
  cl_mem bufB = clCreateBuffer( context, CL_MEM_READ_ONLY, datasize, NULL, &status );
  cl_mem bufC = clCreateBuffer( context, CL_MEM_WRITE_ONLY, datasize, NULL, &status );

  //Write data from the input arrays to the buffers
  status = clEnqueueWriteBuffer(cmdQueue, bufA, CL_FALSE, 0, datasize, A, 0, NULL, NULL);
  status = clEnqueueWriteBuffer(cmdQueue, bufB, CL_FALSE, 0, datasize, B, 0, NULL, NULL);

  //Create a program with source code
  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&programSource, NULL, &status);

  //Build/compile the program for the device
  status = clBuildProgram(program, 1, device[0], NULL, NULL, NULL);

  //create the vector addition kernel
  cl_kernel kernel = clCreateKernel(program, "vecadd", &status);

  //Set the kernel arguments
  status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
  status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
  status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);

  //define an index space of work-items for execution.
  //A work-group size is not required, but can be used
  size_t indexSpaceSize[1], workGroupSize[1];

  //There are 'elements' work-items
  indexSpaceSize[0] = elements;
  workGroupSize[0] = 256;

  //Execute the kernel
  status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, indexSpaceSize, workGroupSize, 0, NULL, NULL);

  //Read the device output buffer to the host output array
  status = clEnqueueReadBuffer(cmdQueue, bufC, CL_TRUE, 0, datasize, C, 0, NULL, NULL);

  //Freee OpenCL resources
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(cmdQueue);
  clReleaseMemObject(bufA);
  clReleaseMemObject(bufB);
  clReleaseMemObject(bufC);
  clReleaseContext(context);

  //Free host resources
  free(A);
  free(B);
  free(C);

  return 0;


	//getchar();
}