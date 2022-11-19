#ifndef JPEG_HPP
#define JPEG_HPP
#include <cstddef>
#include <cstdio>
#include <jpeglib.h>
#include <string>
#include <csetjmp>

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */




extern "C" int jpeg_initialize (jpeg_decompress_struct *cinfo);
extern "C" int read_JPEG_file (const char *data, size_t length, struct jpeg_decompress_struct *cinfo,
                               JSAMPARRAY *output, struct my_error_mgr *jerr);


class jpeg{
private:
    bool allocated = false;
    jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPARRAY image_memory;
public:
    jpeg();
    const unsigned char * const * const image_memory_ptr() {return (const unsigned char**)image_memory;};

    bool decode(const std::string &s);
    ~jpeg();
    inline int height(){return cinfo.output_height;};
    inline int width(){return cinfo.output_width;};
    inline int components(){return cinfo.output_components;};

};

#endif // JPEG_HPP
