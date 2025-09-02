#include <iostream>

#include "../include/bitmap.h"

int main(int argc, char *argv[])
{
    bmp::image im = bmp::read_bmp("bitmaps/image.bmp");
    std::cout << im.img_data.size() << std::endl;
}