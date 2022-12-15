#include <sstream>
#include <iostream>
#include "jpeg.hpp"
#include <cstring>


jpeg::jpeg()
{
//    cinfo = new jpeg_decompress_struct;
}

jpeg::jpeg ( jpeg && j)
{
    allocated = j.allocated;
    cinfo = j.cinfo;
    jerr = j.jerr;
    image_memory = j.image_memory;
    j.allocated = false;
}

bool jpeg::decode(const std::string &s)
{
    if (allocated)
    {
        return false;
//        jpeg_finish_decompress(cinfo);
//        allocated = false;
    }
    if (!jpeg_initialize(&cinfo)) return false;
    if (read_JPEG_file(s.c_str(), s.size(), &cinfo, &image_memory, &jerr))
        allocated = true;
    else return false;
    return true;
}

jpeg::~jpeg()
{
    /*
    if (allocated)
    {
        jpeg_finish_decompress(&cinfo);
    }
    */
    if (allocated) jpeg_destroy_decompress(&cinfo);
//    delete cinfo;
}
