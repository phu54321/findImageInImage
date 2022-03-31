#include <stdio.h>
#include <stdint.h>

const uint32_t haystack[10][10] = {
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
    {11, 12, 13, 14, 15, 16, 17, 18, 19, 20},
    {21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
    {31, 32, 33, 34, 35, 36, 37, 38, 39, 40},
    {41, 42, 43, 44, 45, 46, 47, 48, 49, 50},
    {51, 52, 53, 54, 55, 56, 57, 58, 59, 60},
    {61, 62, 63, 64, 65, 66, 67, 68, 69, 70},
    {71, 72, 73, 74, 75, 76, 77, 78, 79, 80},
    {81, 82, 83, 84, 85, 86, 87, 88, 89, 90},
    {91, 92, 93, 94, 95, 96, 97, 98, 99, 100},
};

const uint32_t needle[2][3] = {
    {45, 46, 47},
    {55, 56, 57}};

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

void calculateRollingHash(
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
  for ( i = 0 ; i < windowSize ; i++) {
    hash = (hash * hashMultiplier) + *input;
    input += inputPitch;
  }

  // To support overlapping input/output, we store output after rolling the hash.
  tmp = hash;
  hash -= hashMultiplierPowWindowSize * input[-inputPitch * windowSize];
  *output = tmp;
  output += outputPitch;

  // Calculate output for the rest of the range.
  for ( i = windowSize ; i < inputSize ; i++) {
    hash = (hash * hashMultiplier) + *input;
    input += inputPitch;

    tmp = hash;
    hash -= hashMultiplierPowWindowSize * input[-inputPitch * windowSize];
    *output = tmp;
    output += outputPitch;
  }
}

int main()
{
  uint32_t xHashes[2];
  calculateRollingHash(3, hashXMultiplier, needle[0], 1, 3, &xHashes[0], 1);
  calculateRollingHash(3, hashXMultiplier, needle[1], 1, 3, &xHashes[1], 1);

  uint32_t searchHash;
  calculateRollingHash(2, hashYMultiplier, xHashes, 1, 2, &searchHash, 1);

  // hash should be 0x892E8B32
  printf("search: %08X\n", searchHash);

  uint32_t haystackHashMap[10][8] = {0};
  for (int y = 0 ; y < 10 ; y++) {
    calculateRollingHash(3, hashXMultiplier, haystack[y], 1, 10, haystackHashMap[y], 1);
  }

  for (int x = 0 ; x < 8 ; x++) {
    calculateRollingHash(2, hashYMultiplier, &haystackHashMap[0][x], 8, 10, &haystackHashMap[0][x], 8);
  }

  for (int y = 0 ; y < 9 ; y++) {
    for(int x = 0 ; x < 8 ; x ++) {
      printf("%08X  ", haystackHashMap[y][x]);
      if (haystackHashMap[y][x] == searchHash) {
        printf("\nfound: (%d, %d)\n", x, y);
      }
    }
    printf("\n");
  }

  return 0;
}
