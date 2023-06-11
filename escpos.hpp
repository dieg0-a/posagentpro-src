#ifndef ESCPOS_HPP
#define ESCPOS_HPP
#include <cstddef>
#include <sstream>
#include "jpeg.hpp"


class escpos
{
private:

    std::stringstream buf;

public:
    escpos();
    escpos &begin();
//    escpos &image_from_base64_encoded_jpeg(std::string &s);
    escpos &image_from_jpeg(const std::string &s);
    escpos &image_from_jpeg(const jpeg &jpeg_object);

    escpos &image_from_bitmap(const unsigned char * const * const s, int width, int height, int bytespp);
    escpos &image_from_bitmap_demo(const unsigned char * const * const s, int width, int height, int bytespp);
    escpos &feednlines(int n);
    escpos &fullcut();
    escpos &cashdrawer();


    const std::string end();
    short max_width = 576;
    int gamma = 240;
    std::string protocol_type = "escpos";
};


#endif // ESCPOS_HPP
