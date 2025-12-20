#include "tgaimage.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <ratio>
#include <string>
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
    if (!decompress_rle_encoded_data(in)) {
      std::cerr << "An error occured while reading data" << std::endl;
      return false;
    }
  } else {
    std::cerr << "Unknown File Format" << static_cast<int>(header.imagetypecode)
              << std::endl;
    return false;
  }

  if (!(header.imagedescriptor & 0x20))
    flip_vertically();
  if (header.imagedescriptor & 0x10)
    flip_horizontally();

  std::cerr << w << "x" << h << "/" << bpp * 8 << std::endl;
  return true;
}

bool TGAImage::decompress_rle_encoded_data(std::ifstream &in) {
  size_t entire_image_pixel_count = w * h;
  size_t current_pixel = 0;
  size_t current_byte = 0;
  std::uint8_t run_count;
  TGAColor color_buffer;

  do {
    std::uint8_t header_chunk = 0;
    header_chunk = in.get();

    if (!in.good()) {
      std::cerr << "An error occured while reading header_chunk from the "
                   "encoded input "
                   "file stream"
                << std::endl;
      return false;
    }

    if (header_chunk < 128) {
      run_count = ++header_chunk;
      for (int i{0}; i < run_count; i++) {
        in.read(reinterpret_cast<char *>(color_buffer.bgra), bpp);
        if (!in.good()) {
          std::cerr << "An error occured while reading the literal run from "
                       "the input file stream"
                    << std::endl;
          return false;
        }

        for (int pixel_channel{0}; pixel_channel < bpp; pixel_channel++) {
          data[current_byte++] = color_buffer.bgra[pixel_channel];
        }

        current_pixel++;
        if (current_pixel > entire_image_pixel_count) {
          std::cerr << "Too many pixel read" << std::endl;
          return false;
        }
      }
    } else {
      run_count = header_chunk - 127;

      in.read(reinterpret_cast<char *>(color_buffer.bgra), bpp);
      if (!in.good()) {
        std::cerr << "An error occured while reading the encoded run from the "
                     "input file stream"
                  << std::endl;
        return false;
      }

      for (int i{0}; i < run_count; i++) {
        for (int pixel_channel{0}; pixel_channel < bpp; pixel_channel++) {
          data[current_byte++] = color_buffer.bgra[pixel_channel];
        }
        current_pixel++;
        if (current_pixel > entire_image_pixel_count) {
          std::cerr << "Too many pixel read" << std::endl;
          return false;
        }
      }
    }
  } while (current_pixel < entire_image_pixel_count);

  return true;
}

bool TGAImage::write_tga_file(const std::string filename, const bool vflip,
                              const bool rle) const {
  TGAFooter footer;
  std::ofstream out;
  out.open(filename, std::ios::binary);
  if (!out.is_open()) {
    std::cerr << "Can't open file " << filename << std::endl;
    return false;
  }
  TGAHeader header = {};
  header.bitsperpixel = bpp << 3;
  header.width = w;
  header.height = h;
  header.imagetypecode =
      (bpp == GRAYSCALE
           ? (rle ? COMPRESSED_GRAY_IMAGE_TYPE_CODE : GRAY_IMAGE_TYPE_CODE)
           : (rle ? COMPRESSED_RGB_IMAGE_TYPE_CODE : RGB_IMAGE_TYPE_CODE));
  header.imagedescriptor =
      vflip ? 0x00 : 0x20; // top-left or bottom-left origin

  out.write(reinterpret_cast<const char *>(&header), sizeof(header));
  if (!out.good())
    goto err;
  if (!rle) {
    out.write(reinterpret_cast<const char *>(data.data()), w * h * bpp);
    if (!out.good())
      goto err;
  } else if (!unload_rle_data(out))
    goto err;

  out.write(reinterpret_cast<const char *>(footer.developer_area_ref),
            sizeof footer.developer_area_ref);
  if (!out.good())
    goto err;

  out.write(reinterpret_cast<const char *>(footer.extension_area_ref),
            sizeof footer.extension_area_ref);
  if (!out.good())
    goto err;

  out.write(reinterpret_cast<const char *>(footer.signature),
            sizeof footer.signature);
  if (!out.good())
    goto err;

  return true;
err:
  std::cerr << "Can't dump the TGA file" << std::endl;
  return false;
}

bool TGAImage::unload_rle_data(std::ofstream &out) const {
  const std::uint8_t maxChunkLength = 128;
  size_t entireImagePixelCount = w * h;
  size_t currentPixel = 0;

  // Process and unload each pixel until we are
  // out of pixels in the image
  while (currentPixel < entireImagePixelCount) {
    size_t chunkStartPos = currentPixel * bpp;
    size_t currentByte = currentPixel * bpp;
    std::uint8_t runLength = 1;
    bool isRawLiteral = true;

    while (currentPixel + runLength < entireImagePixelCount &&
           runLength < maxChunkLength) {
      bool samePixels = true;
      for (int t{0}; samePixels && t < bpp; t++) {
        std::uint8_t currentPixelData = data[currentByte + t];
        std::uint8_t nextPixelData = data[currentByte + t + bpp];
        samePixels = (currentPixelData == nextPixelData);
      }
      currentByte += bpp;
      if (runLength == 1)
        isRawLiteral = !samePixels;
      if (isRawLiteral && samePixels) {
        runLength--;
        break;
      }
      if (!isRawLiteral && !samePixels) {
        break;
      }
      runLength++;
    }
    currentPixel += runLength;
    out.put(isRawLiteral ? runLength - 1 : runLength + 127);
    if (!out.good())
      return false;
    out.write(reinterpret_cast<const char *>(data.data() + chunkStartPos),
              (isRawLiteral ? runLength * bpp : bpp));
    if (!out.good())
      return false;
  }

  return true;
}

TGAColor TGAImage::get(const int x, const int y) const {
  if (!data.size() || x < 0 || y < 0 || x >= w || y >= h)
    return {};
  TGAColor ret = {0, 0, 0, 0, bpp};
  const std::uint8_t *p = data.data() + (x + y * w) * bpp;
  for (int i = bpp; i--; ret.bgra[i] = p[i])
    ;
  return ret;
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

void TGAImage::set(const int x, const int y, const TGAColor &c) {
  if (!data.size() || x < 0 || y < 0 || x >= w || y >= h)
    return;
  memcpy(data.data() + (x + y * w) * bpp, c.bgra, bpp);
}

int TGAImage::width() const { return w; }

int TGAImage::height() const { return h; }
