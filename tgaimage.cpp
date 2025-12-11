#include "tgaimage.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

const std::uint8_t RGB_IMAGE_TYPE_CODE = 2;
const std::uint8_t GRAY_IMAGE_TYPE_CODE = 3;

TGAImage::TGAImage(const int w, const int h, const int bpp)
    : w(w), h(h), bpp(bpp), data(w * h * bpp, 0) {}

bool TGAImage::read_tga_file(const std::string filename) {
  std::ifstream in; // Input stream class inctance to operate on the file
  in.open(filename, std::ios::binary);
  if (!in.is_open()) {
    std::cerr << "Can't open file " << std::endl;
    return false;
  }

  TGAHeader header;

  in.read(static_cast<char *>(static_cast<void *>(&header)), sizeof(header));
  if (!in.good()) {
    std::cerr << "An error has occured while reading the header" << std::endl;
    return false;
  }

  w = header.width;
  h = header.height;
  bpp = header.bitsperpixel >> 3; // bpp -> bytes per pixel
                                  // shifting right by 3 --
                                  // the same as deviding by 8 but faster
  if (w <= 0 || h <= 0 || (bpp != GRAYSCALE && bpp != RGB && bpp != RGBA)) {
    std::cerr << "Bad bpp (or width/height) value" << std::endl;
    return false;
  }

  size_t numberOfBytes = bpp * w * h;
  data = std::vector<std::uint8_t>(numberOfBytes, 0);
  if (header.imagetypecode == GRAY_IMAGE_TYPE_CODE ||
      header.imagetypecode == RGB_IMAGE_TYPE_CODE) {
    in.read(static_cast<char *>(static_cast<void *>(data.data())),
            numberOfBytes);
    if (!in.good()) {
      std::cerr << "An error has occured while reading the data" << std::endl;
      return false;
    }
  }
}
