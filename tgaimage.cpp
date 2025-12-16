#include "tgaimage.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <vector>

const std::uint8_t RGB_IMAGE_TYPE_CODE = 2;
const std::uint8_t GRAY_IMAGE_TYPE_CODE = 3;

TGAImage::TGAImage(const int w, const int h, const int bpp)
    : w(w), h(h), bpp(bpp), data(w * h * bpp, 0) {}

bool TGAImage::read_tga_file(const std::string filename) {
  // input file stream: "in"
  std::ifstream in;
  // opening a file through the defnied input file stream
  in.open(filename, std::iostream::binary);

  // check if opening the file failed
  if (!in.is_open()) {
    std::cerr << "Can't open file " << filename << std::endl;
    return false;
  }

  TGAHeader header;
  in.read(reinterpret_cast<char *>(&header), sizeof(header));
  // check quickly one more time to make sure
  // no errors has occured
  if (!in.good()) {
    std::cerr << "An error occured during reading the header" << std::endl;
    return false;
  }

  w = header.width;
  h = header.height;
  // Note: bpp here stands for "'byte' per pixel"
  bpp = header.bitsperpixel >> 3;
  if (w <= 0 || h <= 0 || (bpp != GRAYSCALE && bpp != RGB && bpp != RGBA)) {
    std::cerr << "Bad bpp (or /width/height) value" << std::endl;
    return false;
  }

  size_t numberOfBytes = bpp * w * h;
  data = std::vector<std::uint8_t>(numberOfBytes, 0);
  if (header.imagetypecode == RGB_IMAGE_TYPE_CODE ||
      header.imagetypecode == GRAY_IMAGE_TYPE_CODE) {
    in.read(reinterpret_cast<char *>(data.data()), numberOfBytes);
    if (!in.good()) {
      std::cerr << "An error occured while reading the data" << std::endl;
      return false;
    }
  }
}
