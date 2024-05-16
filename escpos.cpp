#include "escpos.hpp"
#include "jpeg.hpp"
#include <cmath>
#include <string>


int bayer_matrix[] = {1,  33, 9,  41, 3,  35, 11, 43, 49, 17, 57, 25, 51,
                      19, 59, 27, 13, 45, 5,  37, 15, 47, 7,  39, 61, 29,
                      53, 21, 63, 31, 55, 23, 4,  36, 12, 44, 2,  34, 10,
                      42, 52, 20, 60, 28, 50, 18, 58, 26, 16, 48, 8,  40,
                      14, 46, 6,  38, 64, 32, 56, 24, 62, 30, 54, 22};

inline double bayer(int i, int j) {
  return double(bayer_matrix[i * 4 + j]) / 64.0;
}


escpos::escpos(){};

escpos &escpos::image_from_jpeg(const jpeg &jpeg_object, int max_width, int gamma) {
    const unsigned char *const *const s = jpeg_object.image_memory_ptr();
  int width = jpeg_object.width();
  int height = jpeg_object.height();
  int bytespp = jpeg_object.components();

  double ratio = double(width) / double(max_width);
  int new_height = height / ratio;
  short bitcounter = 0;
  unsigned char buffer_byte = '\0';
  for (int i = 0; i < new_height; i++) {
    if (i % 255 == 0) {
      buf << (unsigned char)0x1D << (unsigned char)0x76 << (unsigned char)0x30
          << (unsigned char)0x00;
      buf << (unsigned char)((max_width / 8) % 256)
          << (unsigned char)((max_width / 8) / 256);
      buf << (unsigned char)((new_height - i) < 256 ? (new_height - i) : 255)
          << (unsigned char)0x00;
    }
    for (int j = 0; j < (max_width); j++) {
      /// compute the values of pixel coordinates in source image space

      double i_s = double(i) * ratio;
      double j_s = double(j) * ratio;

      /// compute coordinates of adjacent pixels

      double i_bottom_d = floor(i_s);
      double i_top_d = ceil(i_s) <= height - 1 ? ceil(i_s) : height - 1;
      double j_bottom_d = floor(j_s);
      double j_top_d = ceil(j_s) <= width - 1 ? ceil(j_s) : width - 1;

      int i_bottom = i_bottom_d;
      int i_top = i_top_d;
      int j_bottom = j_bottom_d;
      int j_top = j_top_d;

      double dx = i_s - i_bottom_d;
      double dy = j_s - j_bottom_d;

      int colorbyte = 0;

      for (int k = 0; k < bytespp; k++) {
        double c_bottom_left = s[i_bottom][(j_bottom * bytespp) + k];
        double c_bottom_right = s[i_bottom][(j_top * bytespp) + k];
        double c_top_left = s[i_top][(j_bottom * bytespp) + k];
        double c_top_right = s[i_top][(j_top * bytespp) + k];
        double dcdytop = c_top_right - c_top_left;
        double dcdybottom = c_bottom_right - c_bottom_left;
        double c_top = c_top_left + dcdytop * dy;
        double c_bottom = c_bottom_left + dcdybottom * dy;
        double dcdx = c_top - c_bottom;
        double component = c_bottom + dcdx * dx;
        colorbyte += component;
      }
      colorbyte /= bytespp;

      colorbyte = colorbyte <= 0xFF ? colorbyte : 0xFF;
      // Transform to linear RGB

      double color = double(colorbyte) / 255.0;

      if (color < 0.04045)
        color = color / 12.92;
      else
        color = pow((color + 0.055) / 1.055, (float)gamma / 100.0);

      int colorbyte_linear = color * 255.0;
      if (colorbyte_linear < 0xAA) {
        if (colorbyte_linear > 0x60) {
          if (bayer(i % 4, j % 4) * 255 > colorbyte_linear) {
            buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));
          }
        } else
          buffer_byte += (unsigned char)(0x01 << (7 - bitcounter));
      }
      //           else buffer_byte += (unsigned char)(0x01 << (7-bitcounter));

      bitcounter++;
      if (bitcounter == 8) {
        bitcounter = 0;
        buf << buffer_byte;
        buffer_byte = 0;
      }
    }
  }
  return *this;

}

escpos &escpos::image_from_jpeg(const std::string &str, int max_width, int gamma) {
  jpeg image;
  if (!image.decode(str)) return *this;

  return image_from_jpeg(image, max_width, gamma);
}


escpos &escpos::feednlines(int n) {
  for (int i = 0; i < n; i++)
    buf << "\r\n";
  return *this;
}

escpos &escpos::fullcut() {
  buf << (unsigned char)0x1B << (unsigned char)0x69;
  return *this;
}

escpos &escpos::cashdrawer() {
  //    if (protocol_type == ESCPOS)
  //    {
  buf << (unsigned char)0x1B << (unsigned char)0x3D << (unsigned char)0x01;
  buf << (unsigned char)0x1B << (unsigned char)0x70 << (unsigned char)0x00
      << (unsigned char)0x19 << (unsigned char)0x19;
  buf << (unsigned char)0x1B << (unsigned char)0x70 << (unsigned char)0x01
      << (unsigned char)0x19 << (unsigned char)0x19;
  //    }
  return *this;
}

escpos &escpos::begin() {
  buf.str(std::string());
  return *this;
}

const std::string escpos::end() { return buf.str(); }
