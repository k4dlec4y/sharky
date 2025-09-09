#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <vector>
#include <fstream>

constexpr std::size_t HIDDEN_METADATA_SIZE = 8;

namespace bmp {

struct image {
    std::string filename;
    std::ifstream in;
    std::ofstream out;
    int32_t width;
    int32_t height;
    int16_t channel_count;

    int32_t data_offset;
    /* how many bytes can be hidden in total */
    std::size_t byte_capacity;
    int padding;
    /* used for extraction */
    unsigned char id;
    unsigned char seq;

    std::vector<unsigned char> header;

    image(const char *filename) : filename(filename),
        in(filename, std::ios::binary) {}

    image(std::string filename) : filename(filename),
        in(filename, std::ios::binary) {}

    bool open_ofstream();
};

/**
 * Reads, checks and stores relevant information about bmp image into image
 * struct.
 * 
 * @param im reference to image struct, where information is stored
 * @param chunk_count amount of bytes needed to hide a single byte into
 * the image, used to count byte_capacity of the image
 * 
 * @return `true` on success, `false` otherwise
 * 
 * @note image struct `im` must have opened ifstream `in` (this should be
 * guaranteed by the image constructor) currently pointing at the beginning
 * of the file. After the call, ifstream `in` is pointing at beginning
 * of bmp's data section.
 * 
 */
bool read_header(image &im, std::size_t chunk_count);

}

#endif  // BITMAP_H
