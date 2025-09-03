#include <iostream>

#include "../include/bitmap.h"
#include "../include/hide.h"

std::string text =
    "In programming, clarity often outweighs cleverness. Readable code is easier "
    "to maintain, review, and debug, especially when working in teams or revisiting "
    "a project after months or years. While concise expressions may feel elegant, "
    "they should not come at the cost of comprehension. A good rule of thumb is to "
    "write code as though the next person maintaining it is a tired version of yourself.";

int main()
{
    bmp::image im = bmp::read_bmp("bitmaps/image.bmp");
    std::cout << im.width << std::endl;
    std::cout << im.height << std::endl;
    std::cout << im.byte_capacity << std::endl;
    std::cout << im.padding << std::endl;
    std::cout << im.img_data.size() << std::endl;

    hide_data(im, reinterpret_cast<unsigned char *>(text.data()), text.size(), 2);

    return bmp::write_bmp(im) ? 0 : -1;
}
