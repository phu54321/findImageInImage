/**
 * Copyright (c) 2022 Hyun Woo Park
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include <stdint.h>

/* two prime value whose multiplication can efficiently be computed */
const uint32_t hashXMultiplier = 65537;
const uint32_t hashYMultiplier = 257;

/**
 * Utility functions
 */
static uint32_t uint32pow(uint32_t a, size_t b)
{
  uint32_t ret = 1;
  while (b)
  {
    if (b & 1)
    {
      ret *= a;
    }
    a *= a;
    b >>= 1;
  }
  return ret;
}

/* //////////////////////////////////////////////////// */

static void calculateRollingHash(
    uint32_t windowSize,
    uint32_t hashMultiplier,

    const uint32_t *input,
    size_t inputPitch,
    uint32_t inputSize,

    uint32_t *output,
    size_t outputPitch)
{
  uint32_t hash = 0, tmp;
  const uint32_t hashMultiplierPowWindowSize = uint32pow(hashMultiplier, windowSize - 1);
  size_t i;

  // Calculate iniial hash for input[0 ~ inputSize - 1]
  for (i = 0; i < windowSize; i++)
  {
    hash = (hash * hashMultiplier) + *input;
    input += inputPitch;
  }

  // To support overlapping input/output, we store output after rolling the hash.
  tmp = hash;
  hash -= hashMultiplierPowWindowSize * input[-inputPitch * windowSize];
  *output = tmp;
  output += outputPitch;

  // Calculate output for the rest of the range.
  for (i = windowSize; i < inputSize; i++)
  {
    hash = (hash * hashMultiplier) + *input;
    input += inputPitch;

    tmp = hash;
    hash -= hashMultiplierPowWindowSize * input[-inputPitch * windowSize];
    *output = tmp;
    output += outputPitch;
  }
}

// ****************

void find2DImageInImage(
    const uint32_t *haystackImage,
    size_t haystackW,
    size_t haystackH,
    size_t haystackPitch,

    const uint32_t *needleImage,
    size_t needleW,
    size_t needleH,
    size_t needlePitch,

    void (*callback)(size_t x, size_t y))
{
  // assert(needleH <= haystackH);
  // assert(needleW <= haystackW);

  size_t searchSpaceW = haystackW - needleW + 1;
  size_t searchSpaceH = haystackH - needleH + 1;

  uint32_t *hashBuffer = (uint32_t *)malloc(sizeof(uint32_t) * searchSpaceW * haystackH);
  uint32_t searchHash;
  size_t x, y;

  // 1) calculate hash of needle
  for (y = 0; y < needleH; y++)
  {
    calculateRollingHash(
        needleW, hashXMultiplier,
        needleImage + needlePitch * y, 1, needleW,
        hashBuffer + searchSpaceW * y, 1);
  }

  calculateRollingHash(
      needleH, hashYMultiplier,
      hashBuffer, searchSpaceW, needleH,
      &searchHash, 1);

  // 2) calculate rolling hash of haystacks
  for (y = 0; y < haystackH; y++)
  {
    calculateRollingHash(
        needleW, hashXMultiplier,
        haystackImage + haystackPitch * y, 1, haystackW,
        hashBuffer + searchSpaceW * y, 1);
  }

  for (x = 0; x < searchSpaceW; x++)
  {
    calculateRollingHash(
        needleH, hashYMultiplier,
        hashBuffer + x, searchSpaceW, haystackH,
        hashBuffer + x, searchSpaceW);
  }

  // 3) Search within
  const uint32_t *hashSearcher = hashBuffer;
  for (y = 0; y < searchSpaceH; y++)
  {
    for (x = 0; x < searchSpaceW; x++)
    {
      if (*hashSearcher == searchHash)
      {
        // Hash matches. Perform memcmp
        size_t ix, iy;
        int exactMatch = 1;
        for (iy = 0 ; exactMatch && iy < needleH ; iy++) {
          for(ix = 0 ; ix < needleW ; ix++) {
            if (needleImage[iy * needlePitch + ix] != haystackImage[(y + iy) * haystackPitch + (x + ix)]) {
              exactMatch = 0;
              break;
            }
          }
        }
        if (exactMatch) {
          callback(x, y);
        }
      }
      hashSearcher++;
    }
  }

  free(hashBuffer);
}
