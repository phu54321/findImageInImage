/**
 * Copyright (c) 2022 Hyun Woo Park
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "findImageInImage.h"
#include "lodepng.h"
#include <time.h>

void decodeOneStep(const char *filename)
{
  unsigned error;
  unsigned char *image = 0;
  unsigned width, height;

  error = lodepng_decode32_file(&image, &width, &height, filename);
  if (error)
    printf("error %u: %s\n", error, lodepng_error_text(error));

  /*use image here*/

  free(image);
}

void cb(size_t x, size_t y)
{
  printf("found (%lu, %lu)\n", x, y);
}

int main()
{
  unsigned char *haystack;
  unsigned haystackW, haystackH;
  unsigned char *needle;
  unsigned needleW, needleH;
  clock_t t0;

  lodepng_decode32_file(&haystack, &haystackW, &haystackH, "../testData/screenshot.png");
  lodepng_decode32_file(&needle, &needleW, &needleH, "../testData/inspector_transform.png");
  printf("haystack size: %u x %u\n", haystackW, haystackH);
  printf("needle size: %u x %u\n", needleW, needleH);

  t0 = clock();
  find2DImageInImage(
      (uint32_t *)haystack, haystackW, haystackH, haystackW,
      (uint32_t *)needle, needleW, needleH, needleW,
      cb);
  printf(" - elapsed time: %lfms\n", (double)(clock() - t0) / CLOCKS_PER_SEC);

  t0 = clock();
  find2DImageInImage(
      (uint32_t *)haystack, haystackW, haystackH, haystackW,
      (uint32_t *)needle, needleW, needleH, needleW,
      cb);
  printf(" - elapsed time: %lfms\n", (double)(clock() - t0) / CLOCKS_PER_SEC);

  // 0.034909ms in m1 pro. YMMV

  free(haystack);
  free(needle);

  return 0;
}
