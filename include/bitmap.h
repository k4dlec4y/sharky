#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <vector>

constexpr std::size_t HIDDEN_METADATA_SIZE = 8;

namespace bmp {

struct image {
    std::string filename;
    int32_t width;
    int32_t height;
    int16_t channel_count;
    std::size_t byte_capacity;
    int padding;

    std::vector<unsigned char> header;
    std::vector<unsigned char> img_data;

    image(const char *filename) : filename(filename) {}
};

struct bad_format {
    std::string filename;
    std::string message;
};

image read_bmp(const char *filename);

bool write_bmp(image &im);

}

#endif  // BITMAP_H
