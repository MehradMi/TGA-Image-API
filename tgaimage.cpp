#include "tgaimage.h"
#include <cstddef>
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

  if (!(header.imagedescriptor & 0x20))
    flip_vertically();
  if (header.imagedescriptor & 0x10)
    flip_horizontally();

  std::cerr << w << "x" << h << "/" << bpp * 8 << std::endl;
  return true;
}

bool TGAImage::load_rle_data(std::ifstream &in) {
  size_t entireImagePixelCount = w * h;
  size_t currentPixel = 0;
  size_t currentByte = 0;
  TGAColor colorBuffer;

  do {
    std::uint8_t runCount = 0;
    runCount = in.get(); // Read a single character (one byte) from the
                         // input file stream at a time
    if (!in.good()) {
      std::cerr << "Ar error occured while reading the data" << std::endl;
      return false;
    }

    // if runCount is less than 128, it means
    // that the most significant bit (bit-7) was set to 0
    // and that indicates a "literal run". However, if it is greater
    // than 128, then it means the bit-7 was set to 1, to indicate
    // an "encoded run".
    if (runCount < 128) {
      runCount++;
      for (int i{0}; i < runCount; i++) {
        in.read(reinterpret_cast<char *>(colorBuffer.bgra), bpp);
        if (!in.good()) {
          std::cerr << "An error occured while reading the RLE run"
                    << std::endl;
          return false;
        }
        // Now, take the values stored in the colorBuffer
        // and put them in the "data" vector contiguously
        // as this has the effect of forming the image with
        // correct colors
        for (int t{0}; t < bpp; t++) {
          data[currentByte++] = colorBuffer.bgra[t];
        }

        // Now we can go one step forward and
        // process the next pixel in the image
        currentPixel++;
        if (currentPixel > entireImagePixelCount) {
          std::cerr << "Too many pixel read" << std::endl;
          return false;
        }
      }
    } else {
      runCount -= 127; // Because the last bit was just an indicator (flag) and
                       // should not be present in the runCount
      in.read(reinterpret_cast<char *>(colorBuffer.bgra), bpp);
      if (!in.good()) {
        std::cerr << "An error occured while reading the RLE run" << std::endl;
        return false;
      }
      for (int i{0}; i < runCount; i++) {
        for (int t{0}; t < bpp; t++) {
          data[currentByte++] = colorBuffer.bgra[t];
        }
        currentPixel++;
        if (currentPixel > entireImagePixelCount) {
          std::cerr << "Too many pixel read" << std::endl;
          return false;
        }
      }
    }
  } while (currentPixel < entireImagePixelCount);

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
