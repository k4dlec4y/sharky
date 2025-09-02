#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <vector>

namespace bmp {

struct image {
    std::string filename;
    int32_t width;
    int32_t height;
    int16_t channel_count;
    /* in bits */
    int64_t capacity;
    int padding;

    std::vector<unsigned char> header;
    std::vector<unsigned char> img_data;

    image(const char *filename) : filename(filename) {}
};

image read_bmp(const char *filename);

}

#endif  // BITMAP_H
