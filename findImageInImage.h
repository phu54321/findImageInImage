#pragma once

#ifndef FINDIMAGEINIMAGE_HEADER
#define FINDIMAGEINIMAGE_HEADER

#include <stdint.h>

void find2DImageInImage(
    const uint32_t *haystackImage,
    size_t haystackW,
    size_t haystackH,
    size_t haystackPitch,

    const uint32_t *needleImage,
    size_t needleW,
    size_t needleH,
    size_t needlePitch,

    void (*callback)(size_t x, size_t y));

#endif
