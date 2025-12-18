#include "tgaimage.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <istream>
#include <ratio>
#include <system_error>
#include <vector>

const std::uint8_t RGB_IMAGE_TYPE_CODE = 2;
const std::uint8_t GRAY_IMAGE_TYPE_CODE = 3;
const std::uint8_t COMPRESSED_RGB_IMAGE_TYPE_CODE = 10;
const std::uint8_t COMPRESSED_GRAY_IMAGE_TYPE_CODE = 11;

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
  // read from "in", and store it in &header.
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

  size_t numberOfBytesInImage = bpp * w * h;
  data = std::vector<std::uint8_t>(numberOfBytesInImage, 0);
  if (header.imagetypecode == RGB_IMAGE_TYPE_CODE ||
      header.imagetypecode == GRAY_IMAGE_TYPE_CODE) {
    in.read(reinterpret_cast<char *>(data.data()), numberOfBytesInImage);
    if (!in.good()) {
      std::cerr << "An error occured while reading the data" << std::endl;
      return false;
    }
  } else if (header.imagetypecode == COMPRESSED_RGB_IMAGE_TYPE_CODE ||
             header.imagetypecode == COMPRESSED_GRAY_IMAGE_TYPE_CODE) {
    if (!load_rle_data(in)) {
      std::cerr << "An error occured while reading data" << std::endl;
      return false;
    }
  } else {
    std::cerr << "Unknown File Format" << static_cast<int>(header.imagetypecode)
              << std::endl;
    return false;
  }

  return true;
}

bool TGAImage::load_rle_data(std::ifstream &in) {
  size_t pixelCount = w * h;
  size_t currentByte = 0;
  size_t currentPixel = 0;
  TGAColor colorBuffer;

  do {
    std::uint8_t chunkHeader = 0;
    chunkHeader = in.get(); // Reading a single character of the "in" file
                            // stream at a time
    if (!in.good()) {
      std::cerr << "An error occured while reading the data" << std::endl;
      return false;
    }
    if (chunkHeader < 128) {
      chunkHeader++;
      for (int i{0}; i < chunkHeader; i++) {
        in.read(reinterpret_cast<char *>(colorBuffer.bgra), bpp);
        if (!in.good()) {
          std::cerr << "An error occured while reading the header" << std::endl;
          return false;
        }
        for (int t{0}; t < bpp; t++) {
          data[currentByte++] = colorBuffer.bgra[t];
        }
        currentPixel++;
        if (currentPixel > pixelCount) {
          std::cerr << "Too many pixels read" << std::endl;
          return false;
        }
      }
    } else {
      chunkHeader -= 127;
      for (int i{0}; i < chunkHeader; i++) {
        in.read(reinterpret_cast<char *>(colorBuffer.bgra), bpp);
        if (!in.good()) {
          std::cerr << "An error occured while reading the header" << std::endl;
          return false;
        }
        for (int t{0}; t < bpp; t++) {
          data[currentPixel++] = colorBuffer.bgra[t];
        }
        currentPixel++;
        if (currentPixel > pixelCount) {
          std::cerr << "Too many pixels read" << std::endl;
          return false;
        }
      }
    }
  } while (currentPixel < pixelCount);

  return true;
}

void TGAImage::flip_horizontally() {
  for (int i{0}; i < (w / 2); i++) {
    for (int j{0}; j < h; j++) {
      int currentPixelIndex = i + (j * w);
      int currentPixelByteOffset = currentPixelIndex * bpp;
      int targetPixelIndex = (w - 1 - i) + (j * w);
      int targetPixelByteOffset = targetPixelIndex * bpp;

      for (int channelByteIndex{0}; channelByteIndex < bpp;
           channelByteIndex++) {
        std::swap(data[currentPixelByteOffset + channelByteIndex],
                  data[targetPixelByteOffset + channelByteIndex]);
      }
    }
  }
}

void TGAImage::flip_vertically() {
  for (int i{0}; i < w; i++) {
    for (int j{0}; j < (h / 2); j++) {
      int currentPixelIndex = i + (j * w);
      int currentPixelByteOffset = currentPixelIndex * bpp;
      int targetPixelIndex = i + ((h - 1 - j) * w);
      int targetPixelByteOffset = targetPixelIndex * bpp;
      for (int channelByteIndex{0}; channelByteIndex < bpp;
           channelByteIndex++) {
        std::swap(data[currentPixelIndex + channelByteIndex],
                  data[targetPixelIndex + channelByteIndex]);
      }
    }
  }
}
