#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push, 1)
// TGA Header Structure -> It is 18 bytes long and 12 properties inside it.
struct TGAHeader {
  // Image ID: 1 Byte
  std::uint8_t idlength = 0; // The length of the a "string"
                             // located after the header
  // =====

  // Color Map Type: 1 Byte
  std::uint8_t colormaptype = 0; // Type of color map: 0 = none, 1 = has palette
  // =====

  // Image Type Code: 1 Byte
  std::uint8_t imagetypecode = 0; // Type of image 0 = none, 1 = indexed,
                                  // 2 = rgb, 3 = gray, +8 = rle packed
  // =====

  // Color Map Specification: 5 Bytes
  std::uint16_t colormaptypestart = 0; // First color map entry in palette
  std::uint16_t colormaplength = 0;    // Number of colors in palette
  std::uint8_t colormapdepth = 0;      // Number of bits per palette entry
                                       // 15, 16, 24, 32
  // =====

  // Image Specification: 10 Bytes
  std::uint16_t x_origin = 0;       // Image X origin
  std::uint16_t y_origin = 0;       // Image Y origin
  std::uint16_t width = 0;          // Image width in pixels
  std::uint16_t height = 0;         // Image height in pixels
  std::uint8_t bitsperpixel = 0;    // Images bits per pixel 8, 16, 24, 32
  std::uint8_t imagedescriptor = 0; // Image descriptor bits (vh flip bits) ->
                                    // Determines Image Origin/Orientation

  // =====
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TGAFooter {
  static constexpr std::uint8_t developer_area_ref[4] = {0, 0, 0, 0};
  static constexpr std::uint8_t extension_area_ref[4] = {0, 0, 0, 0};
  static constexpr std::uint8_t signature[18] = {'T', 'R', 'U', 'E', 'V', 'I',
                                                 'S', 'I', 'O', 'N', '-', 'X',
                                                 'F', 'I', 'L', 'E', '.', '\0'};
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

  TGAColor get(const int x, const int y) const;
  void set(const int x, const int y, const TGAColor &c);

  int width() const;
  int height() const;

private:
  bool load_rle_data(std::ifstream &in);
  bool unload_rle_data(std::ofstream &out) const;

  // Both width (w) and height (h) are measured by "pixels" in this program
  int w = 0, h = 0;

  // bpp: bytes per pixel
  std::uint8_t bpp = 0;

  // the "data" vector is meant to
  // store the raw bytes of the entire image,
  // sequentially,
  std::vector<std::uint8_t> data = {};
};
