#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include <CL/cl.h>

int main(void) 
{
	cl_int status;
	cl_platform_id* platform;
	cl_int NumOfPlatforms;

	status = clGetPlatformIDs(0, NULL, &NumOfPlatforms);
	platform = (cl_platform_id*)malloc(sizeof(cl_platform_id) * NumOfPlatforms);
	status = clGetPlatformIDs(NumOfPlatforms, platform, NULL);
	assert(status == CL_SUCCESS);
	printf("In the system there are %d platforms detected.\n", NumOfPlatforms);

	for (int i = 0; i < NumOfPlatforms; i++)
	{
		cl_device_id* device;
		cl_int NumberOfDevices;
		status = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL, 0, NULL, &NumberOfDevices);
		device = (cl_device_id*)malloc(sizeof(cl_device_id)*NumberOfDevices);
		status = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_ALL, NumberOfDevices, device, NULL);
		printf("In platform %d there are %d device(s):\n", i, NumberOfDevices);

		for (int j = 0; j < NumberOfDevices; j++)
		{
			cl_char vendor_name[1024] = { 0 };
			cl_char device_name[1024] = { 0 };
			size_t returned_size = 0;

			status = clGetDeviceInfo(device[j], CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
			status = clGetDeviceInfo(device[j], CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
			printf("    %s, %s\n", vendor_name, device_name);
		}
	}

	getchar();
}