__constant sampler_t sampler =
    CLK_NORMALIZED_COORDS_TRUE |
    CLK_FILTER_NEAREST |
    CLK_ADDRESS_REPEAT;

__kernel
void rotation(
  __read_only image2d_t inputImage,
  __write_only image2d_t outputImage,
  int imageWidth,
  int imageHeight,
  float theta)
{
  int x = get_global_id(0);
  int y = get_global_id(1);
  float value;
  value = read_imagef(inputImage, sampler, (int2)(x, y)).x;
  write_imagef(outputImage, (int2)(x, y), (value, 0.f, 0.f, 0.f));
}