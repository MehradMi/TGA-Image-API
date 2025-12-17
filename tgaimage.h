#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct TGAHeader {
  std::uint8_t idlength = 0; // The length of the a "string"
                             // located after the header

  std::uint8_t colormaptype = 0; // Type of color map: 0 = none, 1 = has palette
  std::uint8_t imagetypecode = 0; // Type of image 0 = none, 1 = indexed,
                                  // 2 = rgb, 3 = gray, +8 = rle packed

  std::uint16_t colormaptypestart = 0; // First color map entry in palette
  std::uint16_t colormaplength = 0;    // Number of colors in palette
  std::uint16_t colormapdepth = 0;     // Number of bits per palette entry
                                       // 15, 16, 24, 32

  std::uint16_t x_origin = 0;       // Image X origin
  std::uint16_t y_origin = 0;       // Image Y origin
  std::uint16_t width = 0;          // Image width in pixels
  std::uint16_t height = 0;         // Image height in pixels
  std::uint8_t bitsperpixel = 0;    // Images bits per pixel 8, 16, 24, 32
  std::uint8_t imagedescriptor = 0; // Image descriptor bits (vh flip bits)
};
#pragma pack(pop)

struct TGAColor {
  // BGRA: Blue-Green-Red-Alpha
  // Why not RGBA? Because TGA Stores Colors In BGRA Byte Order.
  std::uint8_t bgra[4] = {0, 0, 0, 0};

  // bytespp: bytes per pixel
  std::uint8_t bytespp = 4;

  // overloading the "[]" operator
  std::uint8_t &operator[](const int i) { return bgra[i]; }
};

struct TGAImage {
  enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };

  // Constructors
  TGAImage() = default;                              // Default constructor
  TGAImage(const int w, const int h, const int bpp); // Overloading constructor

  // Public Helper Member Functions
  bool read_tga_file(const std::string filename);
  bool write_tga_file(const std::string filename, const bool vflip = true,
                      const bool rle = true) const;
  void flip_horizontally();
  void flip_vertically();

private:
  bool load_rle_data(std::ifstream &in);
  int w = 0, h = 0;
  std::uint8_t bpp = 0;
  std::vector<std::uint8_t> data = {}; // Why??
};
