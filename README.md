# TGA Image API

A lightweight, pure C++ library for reading, writing, and manipulating TGA (Targa) image files. Built as a foundational graphics library for implementing software renderers.

## Features

- **Read & Write TGA Files**: Support for both uncompressed and RLE-compressed TGA formats
- **Multiple Color Formats**: GRAYSCALE, RGB, and RGBA support
- **Image Manipulation**: Built-in horizontal and vertical flipping
- **Pixel-Level Access**: Direct get/set operations for individual pixels
- **RLE Compression**: Automatic compression/decompression for efficient file storage
- **Zero Dependencies**: Pure C++ implementation using only the standard library

## Getting Started

### Building

```bash
g++ -std=c++17 main_test.cpp tgaimage.cpp -o tga_demo
```

### Basic Usage

```cpp
#include "tgaimage.h"

// Create a new image
TGAImage image(100, 100, TGAImage::RGB);

// Set individual pixels
TGAColor red = {0, 0, 255, 255};  // Note: BGRA format
image.set(50, 50, red);

// Write to file
image.write_tga_file("output.tga");

// Read from file
TGAImage loaded_image;
loaded_image.read_tga_file("input.tga");

// Get pixel data
TGAColor pixel = loaded_image.get(50, 50);
```

### Example Output Image
![Example Output Image](frame.png)

## API Reference

### TGAImage Class

#### Constructors
- `TGAImage()` - Default constructor
- `TGAImage(int w, int h, int bpp)` - Create image with specified dimensions and format

#### Methods
- `bool read_tga_file(const std::string filename)` - Load TGA file
- `bool write_tga_file(const std::string filename, bool vflip = true, bool rle = true)` - Save TGA file
- `void set(int x, int y, const TGAColor &c)` - Set pixel color
- `TGAColor get(int x, int y)` - Get pixel color
- `void flip_horizontally()` - Flip image horizontally
- `void flip_vertically()` - Flip image vertically
- `int width()` - Get image width
- `int height()` - Get image height

### TGAColor Structure

```cpp
struct TGAColor {
    std::uint8_t bgra[4];  // Blue, Green, Red, Alpha
    std::uint8_t bytespp;  // Bytes per pixel
};
```

**Note**: TGA uses BGRA byte order, not RGBA.

## Supported Formats

- **Grayscale**: 8-bit single channel
- **RGB**: 24-bit true color
- **RGBA**: 32-bit true color with alpha

Both uncompressed and RLE-compressed variants are supported.

## Project Purpose

This library serves as the image handling foundation for implementing the [tinyrenderer](https://github.com/ssloy/tinyrenderer) project - a software renderer built from scratch. The API provides the essential functionality needed for framebuffer operations and image output in graphics programming.

## Technical Details

- **RLE Compression**: Automatic run-length encoding for reduced file sizes
- **Memory Layout**: Sequential pixel data storage for cache-friendly access
- **Orientation Handling**: Automatic detection and correction of image orientation flags
- **Error Handling**: Comprehensive error checking with stderr logging

## Example Output

The included `main_test.cpp` demonstrates basic usage by creating a 64x64 image with three white pixels at specified coordinates.

## Future Enhancements

- Line drawing algorithms (Bresenham)
- Triangle rasterization
- Texture sampling
- Additional image format support

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests.
