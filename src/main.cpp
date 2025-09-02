#include <iostream>

#include "../include/bitmap.h"

int main()
{
    bmp::image im = bmp::read_bmp("bitmaps/image.bmp");
    std::cout << im.width << std::endl;
    std::cout << im.height << std::endl;
    std::cout << im.capacity << std::endl;
    std::cout << im.padding << std::endl;
    std::cout << im.img_data.size() << std::endl;

    return bmp::write_bmp(im) ? 0 : -1;
}