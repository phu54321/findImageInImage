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
  { 45, 46, 47 },
  { 55, 56, 57 }
};

/* two prime value whose multiplication can efficiently be computed */
const uint32_t hashXMultiplier = 0x65537;
#define multiplyByHashXMultiplier(x) ((x) + ((x) << 16))

const uint32_t hashYMultiplier = 257;
#define multiplyByHashYMultiplier(x) ((x) + ((x) << 8))

/**
 * Calculate simple round hash for image.
 * returns \sum_y=0^hashH { \sum_x=0^hashW { buffer[y * pitch + x] * hashXMultiplier^x * hashYMultiplier^y }}
 */
struct rolling_2d_hash {
  const uint32_t* buffer;
  size_t hashW;
  size_t hashH;
  size_t pitch;
  uint32_t hash;

  /* For computational efficiency */
  uint32_t hashXMultiplierPowHashW;
  uint32_t hashYMultiplierPowHashH;
};

/**
 * Utility functions
 */
static uint32_t uint32pow(uint32_t a, size_t b) {
  uint32_t ret = 1;
  while(b--) {
    ret *= a;
  }
  return ret;
}

/**
 * \sum_x=0^hashW { buffer[x] * hashXMultiplier^x }
 */
static uint32_t calculateRowHash(const uint32_t* buffer, size_t hashW) {
  uint32_t hash = 0;
  const uint32_t *p = buffer, *end = buffer + hashW;
  while(p < end) {
    hash = multiplyByHashXMultiplier(hash) + *(p++);
  }
  return hash;
}

/**
 * \sum_y=0^hashH { buffer[y] * hashYMultiplier^y }
 */
static uint32_t calculateColumnHash(const uint32_t* buffer, size_t hashH, size_t pitch) {
  uint32_t hash = 0;
  const uint32_t *p = buffer, *end = buffer + hashH * pitch;
  while(p < end) {
    hash = multiplyByHashXMultiplier(hash) + *p;
    p += pitch;
  }
  return hash;
}

/* //////////////////////////////////////////////////// */

struct rolling_2d_hash rolling_2d_hash_init(
  const uint32_t* buffer,
  size_t hashW, size_t hashH,
  size_t pitch
) {
  struct rolling_2d_hash ret = {
    buffer,
    hashW,  hashH,
    pitch,
  };

  uint32_t hash = 0;
  const uint32_t *p = buffer, *end = buffer + pitch * hashH;
  while (p < end) {
    hash = multiplyByHashYMultiplier(hash) + calculateRowHash(p, hashW);
    p += pitch;
  }
  ret.hash = hash;

  // Precompute variables
  ret.hashXMultiplierPowHashW = uint32pow(hashXMultiplier, hashW);
  ret.hashYMultiplierPowHashH = uint32pow(hashYMultiplier, hashH);
  return ret;
}

uint32_t hash2d(const uint32_t* buffer, size_t hashW, size_t hashH, size_t pitch) {
  struct rolling_2d_hash hash = rolling_2d_hash_init(buffer, hashW, hashH, pitch);
  return hash.hash;
}


/**
 * Efficiently
 */
void rolling_2d_hash_move_right(
  struct rolling_2d_hash* hash
) {
  hash->hash = multiplyByHashXMultiplier(hash->hash) + calculateColumnHash(hash->buffer + hash->hashW, hash->hashH, hash->pitch);
  hash->hash -= calculateColumnHash(hash->buffer, hash->hashH, hash->pitch) * hash->hashXMultiplierPowHashW;
  hash->buffer++;
}

void rolling_2d_hash_move_down(
  struct rolling_2d_hash* hash
) {
  hash->hash = multiplyByHashYMultiplier(hash->hash) + calculateRowHash(hash->buffer + hash->hashH * hash->pitch, hash->hashW);
  hash->hash -= calculateRowHash(hash->buffer, hash->hashW) * hash->hashYMultiplierPowHashH;
  hash->buffer += hash->pitch;
}


int main() {
  uint32_t searchHash = hash2d(&needle[0][0], 3, 2, 3);
  printf("%08X", searchHash);

  struct rolling_2d_hash yHash = rolling_2d_hash_init(&needle[0][0], 3, 2, 10);
  size_t x, y;
  for(y = 0 ; y < 10 - 2 ; y++) {
    struct rolling_2d_hash xHash = yHash;
    for(x = 0 ; x < 10 - 3 ; x++) {
      printf("found: (%lu, %lu), %08X\n", x, y, xHash.hash);
      // if (xHash.hash == searchHash) {
      //   printf("found: (%lu, %lu), %08X\n", x, y, searchHash);
      // }
      rolling_2d_hash_move_right(&xHash);
    }
    rolling_2d_hash_move_down(&yHash);
  }

  return 0;
}
