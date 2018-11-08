#include<stdio.h>
#include <string.h>
#include "stdlib.h"

#include <CL/cl.h>

#include "utils/utils.h"
#include "utils/bmp-utils.h"

int main(int argc, char **argv)
{
  float *hInputImage = NULL;
  float *hOutputImage = NULL;

  const float theta = 45.0f;

  int imageRows;
  int imageCols;
  hInputImage = readBmpFloat("test.bmp", &imageRows, &imageCols);
  const int imageElements = imageRows * imageCols;
  const size_t imageSize = imageElements * sizeof(float);

  hOutputImage = (float*)malloc(imageSize);
  if (!hOutputImage) { exit(-1); }

  cl_int status;
  cl_platform_id platform;
  status = clGetPlatformIDs(1, &platform, NULL);
  check(status);

  cl_device_id device;
  status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  check(status);

  cl_context context;
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
  check(status);

  cl_command_queue cmdQueue;
  cmdQueue = clCreateCommandQueue(context, device, 0, &status);
  check(status);

  cl_image_desc desc;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = imageCols;
  desc.image_height = imageRows;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.buffer = NULL;

  cl_image_format format;
  format.image_channel_order = CL_R;
  format.image_channel_data_type = CL_FLOAT;

  cl_mem inputImage = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, NULL, NULL);
  cl_mem outputImage = clCreateImage(context, CL_MEM_WRITE_ONLY, &format, &desc, NULL, NULL);

  size_t origin[3] = { 0, 0, 0 };
  size_t region[3] = { imageCols, imageRows, 1 };
  clEnqueueWriteImage(cmdQueue, inputImage, CL_TRUE, origin, region, desc.image_row_pitch, desc.image_slice_pitch, hInputImage, 0, NULL, NULL);

  char *programSource = readFile("imageRotation.cl");
  size_t programSourceLen = strlen(programSource);
  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&programSource, &programSourceLen, &status);
  check(status);

  status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if (status != CL_SUCCESS)
  {
    printCompilerError(program, device);
    exit(-1);
  }

  cl_kernel kernel;
  kernel = clCreateKernel(program, "rotation", &status);
  check(status);

  status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputImage);
  status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputImage);
  status |= clSetKernelArg(kernel, 2, sizeof(int), &imageCols);
  status |= clSetKernelArg(kernel, 3, sizeof(int), &imageRows);
  status |= clSetKernelArg(kernel, 4, sizeof(float), &theta);

  size_t globalWorkSize[2];
  globalWorkSize[0] = imageCols;
  globalWorkSize[1] = imageRows;

  size_t localWorkSize[2];
  localWorkSize[0] = 8;
  localWorkSize[1] = 8;

  status = clEnqueueNDRangeKernel(cmdQueue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
  check(status);

  status = clEnqueueReadImage(cmdQueue, outputImage, CL_TRUE, origin, region, desc.image_row_pitch, desc.image_slice_pitch, hOutputImage, 0, NULL, NULL);
  check(status);

  writeBmpFloat(hOutputImage, "rotatedTest.bmp", imageRows, imageCols, "test.bmp");

  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(cmdQueue);
  clReleaseMemObject(inputImage);
  clReleaseMemObject(outputImage);
  clReleaseContext(context);

  free(hInputImage);
  free(hOutputImage);
  free(programSource);

  return 0;
}
