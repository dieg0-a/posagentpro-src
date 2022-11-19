#include <sstream>
#include <iostream>
#include "jpeg.hpp"
#include <cstring>


jpeg::jpeg()
{
    jpeg_initialize(&cinfo);
}

bool jpeg::decode(const std::string &s)
{
    if (allocated)
    {
        jpeg_finish_decompress(&cinfo);
        allocated = false;
    }
    if (read_JPEG_file(s.c_str(), s.size(), &cinfo, &image_memory, &jerr))
    {
        allocated = true;
        return true;
    }
    else return false;
}

jpeg::~jpeg()
{
    if (allocated)
        jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}
