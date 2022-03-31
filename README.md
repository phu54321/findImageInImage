# findImageInImage

Provides function for searching 2d uint32 image in 2d uint32 image. For use in [Hammerspoon](https://www.hammerspoon.org/).

Function looks like this. `uint32_t` could be any 32-bit value. It's bit enough to match either:

- RGBA8888
- RGB888
- 10-bit RGB values (total 30 bit)

```c
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
```

Function searches exact match of `needleImage` inside `haystackImage`. This funtion variation of [Rabin-Karp](https://en.wikipedia.org/wiki/Rabin%E2%80%93Karp_algorithm) algorithm for faster search. Hashing & hash comparison takes $O(WH)$, where $W$ and $H$ is haystack's width and height.
