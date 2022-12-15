#include "escpos.hpp"
#include "base64.hpp"
#include <string>
#include <iostream>
#include "jpeg.hpp"
#include <fstream>
#include <random>
#include <cmath>
#include "messagesystem.h"


// 'demo', 120x30px
const unsigned char demo [] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff,
    0x80, 0x0f, 0xff, 0xe0, 0x00, 0x07, 0xf8, 0xff, 0xf1, 0xff, 0xf8, 0x00, 0x7f, 0xff, 0xff, 0x80,
    0x01, 0xff, 0xe0, 0x00, 0x07, 0xf8, 0xff, 0xf1, 0xff, 0xf0, 0x00, 0x3f, 0xff, 0xff, 0x80, 0x00,
    0x7f, 0xe0, 0x00, 0x07, 0xf0, 0xff, 0xe1, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x3f,
    0xe0, 0x00, 0x07, 0xf0, 0xff, 0xe1, 0xff, 0xc0, 0xf8, 0x07, 0xff, 0xff, 0x83, 0xe0, 0x1f, 0xe0,
    0xff, 0xff, 0xf0, 0x7f, 0xc1, 0xff, 0x81, 0xfc, 0x07, 0xff, 0xff, 0x87, 0xfc, 0x1f, 0xe0, 0xff,
    0xff, 0xf0, 0x7f, 0xc1, 0xff, 0x03, 0xff, 0x07, 0xff, 0xff, 0x87, 0xfe, 0x0f, 0xe0, 0xff, 0xff,
    0xf0, 0x3f, 0xc0, 0xff, 0x07, 0xff, 0x03, 0xff, 0xff, 0x87, 0xfe, 0x07, 0xe0, 0xff, 0xff, 0xe0,
    0x3f, 0xc0, 0xff, 0x0f, 0xff, 0x81, 0xff, 0xff, 0x87, 0xff, 0x07, 0xe0, 0xff, 0xff, 0xe0, 0x3f,
    0x80, 0xfe, 0x0f, 0xff, 0xc1, 0xff, 0xff, 0x87, 0xff, 0x07, 0xe0, 0xff, 0xff, 0xe0, 0x3f, 0x80,
    0x7e, 0x0f, 0xff, 0xc1, 0xff, 0xff, 0x87, 0xff, 0x07, 0xe0, 0xff, 0xff, 0xe0, 0x1f, 0x80, 0x7e,
    0x0f, 0xff, 0xc1, 0xff, 0xff, 0x87, 0xff, 0x07, 0xe0, 0x00, 0x7f, 0xc0, 0x1f, 0x80, 0x7e, 0x0f,
    0xff, 0xc1, 0xff, 0xff, 0x87, 0xff, 0x03, 0xe0, 0x00, 0x7f, 0xc0, 0x0f, 0x00, 0x7c, 0x0f, 0xff,
    0xc1, 0xff, 0xff, 0x87, 0xff, 0x03, 0xe0, 0x00, 0x7f, 0xc0, 0x0f, 0x00, 0x7c, 0x1f, 0xff, 0xc1,
    0xff, 0xff, 0x87, 0xff, 0x07, 0xe0, 0x00, 0x7f, 0xc3, 0x0e, 0x08, 0x7c, 0x0f, 0xff, 0xc1, 0xff,
    0xff, 0x87, 0xff, 0x07, 0xe0, 0xff, 0xff, 0xc3, 0x0e, 0x18, 0x3c, 0x0f, 0xff, 0xc1, 0xff, 0xff,
    0x87, 0xff, 0x07, 0xe0, 0xff, 0xff, 0xc3, 0x06, 0x18, 0x3e, 0x0f, 0xff, 0xc1, 0xff, 0xff, 0x87,
    0xff, 0x07, 0xe0, 0xff, 0xff, 0x83, 0x86, 0x1c, 0x3e, 0x0f, 0xff, 0xc1, 0xff, 0xff, 0x87, 0xff,
    0x07, 0xe0, 0xff, 0xff, 0x83, 0x80, 0x3c, 0x3e, 0x0f, 0xff, 0xc1, 0xff, 0xff, 0x87, 0xfe, 0x07,
    0xe0, 0xff, 0xff, 0x87, 0x80, 0x3c, 0x1e, 0x0f, 0xff, 0xc1, 0xff, 0xff, 0x87, 0xfe, 0x0f, 0xe0,
    0xff, 0xff, 0x87, 0xc0, 0x7c, 0x1f, 0x0f, 0xff, 0x83, 0xff, 0xff, 0x87, 0xfc, 0x0f, 0xe0, 0xff,
    0xff, 0x07, 0xc0, 0x7c, 0x1f, 0x07, 0xff, 0x03, 0xff, 0xff, 0x87, 0xf8, 0x1f, 0xe0, 0xff, 0xff,
    0x07, 0xe0, 0x7c, 0x1f, 0x03, 0xff, 0x07, 0xff, 0xff, 0x83, 0xe0, 0x1f, 0xe0, 0xff, 0xff, 0x0f,
    0xe0, 0x7e, 0x1f, 0x81, 0xfc, 0x07, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xe0, 0x00, 0x07, 0x0f, 0xe0,
    0xfe, 0x1f, 0x80, 0xf0, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xe0, 0x00, 0x07, 0x0f, 0xf0, 0xfe,
    0x0f, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x01, 0xff, 0xe0, 0x00, 0x07, 0x0f, 0xf0, 0xff, 0x0f,
    0xe0, 0x00, 0x3f, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xe0, 0x00, 0x06, 0x0f, 0xf9, 0xff, 0x0f, 0xf0,
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xfe, 0x07,
    0xff, 0xff
};

int demowidth = 120;
int demoheight = 30;

escpos::escpos(){};
/*
escpos &escpos::image_from_base64_encoded_jpeg(std::string &s)
{
    std::string decoded;
    base64::Decode(s, decoded);
    return image_from_jpeg(decoded);
};
*/

escpos &escpos::image_from_jpeg(const jpeg &jpeg_object)
{
    return image_from_bitmap(jpeg_object.image_memory_ptr(), jpeg_object.width(), jpeg_object.height(), jpeg_object.components());
}


escpos &escpos::image_from_jpeg(const std::string &s)
{
    jpeg image;

    if (image.decode(s))
    {
        return image_from_bitmap(image.image_memory_ptr(), image.width(), image.height(), image.components());
    }
    return *this;
}

int bayer_matrix[] = {1, 33,  9, 41, 3,  35, 11, 43,
                      49, 17, 57, 25, 51, 19, 59, 27,
                      13, 45, 5, 37, 15, 47, 7, 39,
                      61, 29, 53, 21, 63, 31, 55, 23,
                      4, 36, 12, 44, 2, 34, 10, 42,
                      52, 20, 60, 28, 50, 18, 58, 26,
                      16, 48, 8, 40, 14, 46, 6, 38,
                      64, 32, 56, 24, 62, 30, 54, 22};

inline double bayer(int i, int j){
    return double(bayer_matrix[i*4 + j]) / 64.0;
}



inline bool evalpixel(const unsigned char *array, int height, int width)
{
    return demo[( (height*demowidth + width)/8)]
            & (0x01 << (7 - (width)%8));
}

inline void printDemoMark()
{
    for (int i = 0 ; i < demoheight ; i++)
    {
        for (int j = 0 ; j < demowidth ; j++)
        {
            if (evalpixel(demo, i, j)) std::cout << "o";
            else std::cout << " ";
        }
        std::cout << std::endl;
    }
}

escpos &escpos::image_from_bitmap_demo(const unsigned char * const * const s, int width, int height, int bytespp)
{
    double ratio = double(width) / double(max_width);
    int new_height = height/ratio;

    short bitcounter = 0;
    unsigned char buffer_byte = '\0';

    int pixel_counter = 0;

    for (int i = 0; i < new_height ; i++)
    {
        if ( i % 255 == 0)
        {
            buf << (unsigned char)0x1D << (unsigned char) 0x76 << (unsigned char) 0x30 << (unsigned char)0x00;
            buf << (unsigned char)((max_width/8)%256) << (unsigned char)((max_width/8) / 256);
            buf << (unsigned char)((new_height - i) < 256 ? (new_height - i) : 255) << (unsigned char) 0x00;
        }
       for (int j = 0; j < (max_width) ; j++)
       {
           /// compute the values of pixel coordinates in source image space

           if ((j+i) % (demoheight*4) > demoheight*3){
               if ( !evalpixel(demo, ((j+i)%demoheight),  (j)%demowidth))
                   buffer_byte += (unsigned char) (0x01 << (7-bitcounter));
               bitcounter++;
               if (bitcounter == 8)
               {
                   bitcounter = 0;
                   buf << buffer_byte;
                   buffer_byte = 0;
               }
               pixel_counter++;
               continue;
           }

           double i_s = double(i)*ratio;
           double j_s = double(j)*ratio;

           /// compute coordinates of adjacent pixels

           double i_bottom_d = floor(i_s);
           double i_top_d = ceil(i_s) <= height-1 ? ceil(i_s) : height-1;
           double j_bottom_d = floor(j_s);
           double j_top_d = ceil(j_s) <= width-1 ? ceil(j_s) : width-1;

           int i_bottom = i_bottom_d; int i_top = i_top_d; int j_bottom = j_bottom_d; int j_top = j_top_d;

           double dx = i_s - i_bottom_d;
           double dy = j_s - j_bottom_d;

           int colorbyte = 0;

           for (int k = 0 ; k < bytespp ; k++)
           {
               double c_bottom_left = s[i_bottom][(j_bottom*bytespp)+k];
               double c_bottom_right = s[i_bottom][(j_top*bytespp)+k];
               double c_top_left = s[i_top][(j_bottom*bytespp)+k];
               double c_top_right = s[i_top][(j_top*bytespp)+k];

               double dcdytop = c_top_right - c_top_left;
               double dcdybottom = c_bottom_right - c_bottom_left;
               double c_top = c_top_left + dcdytop*dy;
               double c_bottom = c_bottom_left + dcdybottom*dy;

               double dcdx = c_top - c_bottom;

               double component = c_bottom + dcdx * dx;

               colorbyte += component;
           }
           colorbyte /= bytespp;

           colorbyte = colorbyte <= 0xFF ? colorbyte : 0xFF;
           //Transform to linear RGB

           double color = double(colorbyte)/255.0;

           if (color < 0.04045) color = color/12.92;
           else color = pow((color + 0.055)/1.055, float(gamma) / 100.0);

           colorbyte = color*255.0;
           if (colorbyte > 0x20) {
             if (bayer(i % 4, j % 4) * 255 > colorbyte) {
               buffer_byte += (unsigned char)(0x01 << (7-bitcounter));
             }
           }
           else buffer_byte += (unsigned char)(0x01 << (7-bitcounter));

           bitcounter++;
           if (bitcounter == 8)
           {
               bitcounter = 0;
               buf << buffer_byte;
               buffer_byte = 0;
           }
           pixel_counter++;
       }
    }
    return *this;
};

escpos &escpos::image_from_bitmap(const unsigned char * const * const s, int width, int height, int bytespp)
{
    if (GlobalState::demo_mode)
    {
        if (GlobalState::demo_print_jobs < 1) return image_from_bitmap_demo(s, width, height, bytespp);
        else GlobalState::demo_print_jobs--;
    }


    double ratio = double(width) / double(max_width);
    std::cout << "Ratio is: " << ratio << std::endl;
    int new_height = height/ratio;

    std::cout << "Height is: " << height << " and new Height is: " << new_height << std::endl;
    std::cout << "Width is: " << max_width << std::endl;
    std::cout << "Height is: " << new_height << std::endl;

    //    int new_height = 100;
//    buf << (unsigned char)0x1D << (unsigned char) 0x76 << (unsigned char) 0x30 << (unsigned char)0x00;
//    buf << (unsigned char)((max_width/8)%256) << (unsigned char)((max_width/8) / 256);
//    buf << (unsigned char)(new_height % 256) << (unsigned char)(new_height / 256);

    short bitcounter = 0;
    unsigned char buffer_byte = '\0';
    for (int i = 0; i < new_height ; i++)
    {
        if ( i % 255 == 0)
        {
            buf << (unsigned char)0x1D << (unsigned char) 0x76 << (unsigned char) 0x30 << (unsigned char)0x00;
            buf << (unsigned char)((max_width/8)%256) << (unsigned char)((max_width/8) / 256);
            buf << (unsigned char)((new_height - i) < 256 ? (new_height - i) : 255) << (unsigned char) 0x00;
        }
       for (int j = 0; j < (max_width) ; j++)
       {
           /// compute the values of pixel coordinates in source image space

           double i_s = double(i)*ratio;
           double j_s = double(j)*ratio;

           /// compute coordinates of adjacent pixels

           double i_bottom_d = floor(i_s);
           double i_top_d = ceil(i_s) <= height-1 ? ceil(i_s) : height-1;
           double j_bottom_d = floor(j_s);
           double j_top_d = ceil(j_s) <= width-1 ? ceil(j_s) : width-1;

           int i_bottom = i_bottom_d; int i_top = i_top_d; int j_bottom = j_bottom_d; int j_top = j_top_d;

           ///// now compute the weights of each of those pixels
           //Distances
/*
           double btm_left_d = (j_s-j_bottom_d)*(j_s-j_bottom_d) + (i_s-i_bottom_d)*(i_s-i_bottom_d) <= 1 ?
                       (j_s-j_bottom_d)*(j_s-j_bottom_d) + (i_s-i_bottom_d)*(i_s-i_bottom_d) : 1;

           double top_left_d = (j_s-j_top_d)*(j_s-j_top_d) + (i_s-i_bottom_d)*(i_s-i_bottom_d) <= 1 ?
                       (j_s-j_top_d)*(j_s-j_top_d) + (i_s-i_bottom_d)*(i_s-i_bottom_d) : 1;

           double btm_right_d = (j_s-j_bottom_d)*(j_s-j_bottom_d) + (i_s-i_top_d)*(i_s-i_top_d) <= 1 ?
                       (j_s-j_bottom_d)*(j_s-j_bottom_d) + (i_s-i_top_d)*(i_s-i_top_d) : 1;

           double top_right_d = (j_s-j_top_d)*(j_s-j_top_d) + (i_s-i_top_d)*(i_s-i_top_d) <= 1 ?
                       (j_s-j_top_d)*(j_s-j_top_d) + (i_s-i_top_d)*(i_s-i_top_d) : 1;
*/
           double dx = i_s - i_bottom_d;
           double dy = j_s - j_bottom_d;

           //Weights
/*
           double btm_left_w = 1.0 - sqrt(btm_left_d);
           double top_left_w = 1.0 - sqrt(top_left_d);
           double btm_right_w = 1.0 - sqrt(btm_right_d);
           double top_right_w = 1.0 - sqrt(top_right_d);
*/
           //And finally compute the value of each color component based on these weights

           int colorbyte = 0;

           for (int k = 0 ; k < bytespp ; k++)
           {
               double c_bottom_left = s[i_bottom][(j_bottom*bytespp)+k];
               double c_bottom_right = s[i_bottom][(j_top*bytespp)+k];
               double c_top_left = s[i_top][(j_bottom*bytespp)+k];
               double c_top_right = s[i_top][(j_top*bytespp)+k];

               double dcdytop = c_top_right - c_top_left;
               double dcdybottom = c_bottom_right - c_bottom_left;
               double c_top = c_top_left + dcdytop*dy;
               double c_bottom = c_bottom_left + dcdybottom*dy;

               double dcdx = c_top - c_bottom;

               double component = c_bottom + dcdx * dx;





               /*
               component += ( btm_left_w * double(s[i_bottom][(j_bottom*bytespp)+k]) );
               component += ( top_left_w * double(s[i_bottom][(j_top*bytespp)+k]) );
               component += ( btm_right_w * double(s[i_top][(j_bottom*bytespp)+k]) );
               component += ( top_right_w * double(s[i_top][(j_top*bytespp)+k]) );
               */
/*
               int min = 100;
               if (btm_left_d < min){
                   component = s[i_bottom][(j_bottom*bytespp)+k];
                   min = btm_left_d;
               }
               if (top_left_d < min){
                   component = s[i_bottom][(j_top*bytespp)+k];
                   min = top_left_d;
               }
               if (btm_right_d < min){
                   component = s[i_top][(j_bottom*bytespp)+k];
                   min = btm_right_d;
               }
               if (top_right_d < min)
               {
                   component = s[i_top][(j_top*bytespp)+k];
               }
               */

               colorbyte += component;
           }
           colorbyte /= bytespp;

           colorbyte = colorbyte <= 0xFF ? colorbyte : 0xFF;
           //Transform to linear RGB

           double color = double(colorbyte)/255.0;

           if (color < 0.04045) color = color/12.92;
           else color = pow((color + 0.055)/1.055, (float) gamma / 100.0);

           colorbyte = color*255.0;
           if (colorbyte > 0x20) {
             if (bayer(i % 4, j % 4) * 255 > colorbyte) {
               buffer_byte += (unsigned char)(0x01 << (7-bitcounter));
             }
           }
           else buffer_byte += (unsigned char)(0x01 << (7-bitcounter));

           bitcounter++;
           if (bitcounter == 8)
           {
               bitcounter = 0;
               buf << buffer_byte;
               buffer_byte = 0;
           }

       }


    }
    std::cout << "Buffer length at the end of generating pixel data is: " << buf.str().length() << std::endl;
        //    if (bitcounter != 0) {
//      buf << buffer_byte;
    
//    }

//    std::cout << "This image has a width of: " << width << " and a height of: " << height
//              << " and color depth is: " << bytespp << "bpp" << std::endl;
/*
    buf << (unsigned char)0x1D << (unsigned char) 0x76 << (unsigned char) 0x30 << (unsigned char)0x00;
    buf << (unsigned char)((width/8)%256) << (unsigned char)((width/8) / 256);
    buf << (unsigned char)(height % 256) << (unsigned char)(height / 256);
*/

/*
    short bitcounter = 0;
    unsigned char buffer_byte = '\0';
    for (int i = 0; i < new_height ; i++)
    {
       for (int j = 0; j < (max_width*bytespp) ; j+=bytespp)
       {

           unsigned int colorbyte = 0;
           for (int k = 0 ; k < bytespp ; k++)
           {
               colorbyte += s[i][j+k];
           }
           colorbyte /= bytespp;
           if (colorbyte > 0x80){
               if ((unsigned int)(rand() % 0xFF) > colorbyte) {
                   buffer_byte += (unsigned char)(0x01 << (7-bitcounter));
               }
           }
           else buffer_byte += (unsigned char)(0x01 << (7-bitcounter));
           bitcounter++;
           if (bitcounter == 8)
           {
               bitcounter = 0;
               buf << buffer_byte;
               buffer_byte = 0;
           }
       }
    }

*/

    //DEBUG
/*
    std::fstream file;
    try {
        file.exceptions ( std::ofstream::badbit | std::ofstream::failbit );
        file.open("test.bin", std::ios::out);
        file << buf.str();
    }
    catch (const std::ofstream::failure& e) {
        std::cout << "Failure to write binary file test\n";
        if (file.is_open()) file.close();
    }
    if (file.is_open()) file.close();

    //DEBUG
    */


    return *this;
};



escpos& escpos::feednlines(int n)
{
  for (int i = 0; i < n; i++)
    buf << "\r\n";
  return *this;
}

escpos &escpos::fullcut(){
  buf << (unsigned char)0x1B << (unsigned char)0x69;
    return *this;
}

escpos &escpos::cashdrawer()
{
//    if (protocol_type == ESCPOS)
//    {
        buf << (unsigned char) 0x1B << (unsigned char) 0x3D << (unsigned char) 0x01;
        buf << (unsigned char) 0x1B << (unsigned char) 0x70 << (unsigned char) 0x00 << (unsigned char) 0x19 << (unsigned char) 0x19;
        buf << (unsigned char) 0x1B << (unsigned char) 0x70 << (unsigned char) 0x01 << (unsigned char) 0x19 << (unsigned char) 0x19;
//    }
        return *this;
}

escpos &escpos::begin()
{
    buf.str(std::string());
    return *this;
}

const std::string escpos::end()
{
  return buf.str();
}
