#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <vector>
#include <fstream>

const std::size_t hidden_metadata_size = 8;

namespace bmp {

struct image {
    std::string filename;
    std::ifstream input;
    std::ofstream output;
    int32_t width;
    int32_t height;
    /* 3 for BGR, 4 for BGRA*/
    int16_t channel_count;

    int32_t data_offset;
    /* how many bytes can be hidden in total */
    std::size_t byte_capacity;
    /* search "bitmap padding" on the internet for more info */
    int padding;

    /* used for extraction */
    unsigned char id;
    unsigned char seq;

    std::vector<uint8_t> header;

    image(const char *filename) : filename(filename),
        input(filename, std::ios::binary) {}

    image(std::string filename) : filename(filename),
        input(filename, std::ios::binary) {}

    /**
     * Opens an output stream for image '.../image.bmp' as
     * './output_bitmaps/image.bmp.out'
     * 
     * @return `true` on success, `false` otherwise
    */
    bool open_ofstream();
};

/**
 * Reads, checks and stores relevant information about bmp image into image
 * struct.
 * 
 * @param im reference to image struct, where information is stored
 * @param cells_per_byte amount of bytes needed to hide a single byte into
 * the image, used to count byte_capacity of the image
 * 
 * @return `true` on success, `false` otherwise
 * 
 * @note image struct `im` must have opened ifstream `input` (this should be
 * guaranteed by the image constructor) currently pointing at the beginning
 * of the file. After the call, ifstream `input` is pointing at beginning
 * of bmp's data section.
 */
bool load_header(image &im, std::size_t cells_per_byte);

}

#endif  // BITMAP_H
