#pragma once
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct TGAHeader {
  std::uint8_t idlength =
      0; // The length of the a "string" located after the header
  std::uint8_t colormaptype = 0; // Type of color map: 0 = none, 1 = has palette
  std::uint8_t imagetypecode = 0; // Type of image 0 = none, 1 = indexed, 2 =
                                  // rgb, 3 = gray, +8 = rle packed

  std::uint16_t colormaptypestart = 0; // First color map entry in palette
  std::uint16_t colormaplength = 0;    // Number of colors in palette
  std::uint16_t colormapdepth = 0;     // ??
};
