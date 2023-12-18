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
    escpos &image_from_jpeg(const std::string &s, int max_width = 576, int gamma = 240);
    escpos &image_from_jpeg(const jpeg &jpeg_object, int max_width = 576, int gamma = 240);
    escpos &feednlines(int n);
    escpos &fullcut();
    escpos &cashdrawer();
    const std::string end();
};


#endif // ESCPOS_HPP
