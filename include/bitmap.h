#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <vector>
#include <span>
#include <fstream>

#include "../include/configuration.h"

namespace bmp {

struct image {
    std::string filename;
    std::ifstream input;
    std::ofstream output;
    uint32_t width;
    uint32_t height;
    /* 3 for BGR, 4 for BGRA*/
    uint16_t channel_count;

    uint32_t data_offset;
    /* how many bytes can be used for hidding data */
    /* this already excludes size for hidden metadata */
    std::size_t capacity;
    uint8_t chunk_size;
    uint8_t cells_per_byte;
    /* this excludes the size of metadata! */
    std::size_t hidden_data_size{0};
    /* bitmap padding */
    int padding;

    /* used for extraction */
    uint8_t id;
    uint8_t seq;

    std::vector<uint8_t> header;

    image(const char *filename, uint8_t chunk_size) : filename(filename),
        input(filename, std::ios::binary), chunk_size(chunk_size),
        cells_per_byte(8 / chunk_size) {}

    image(std::string filename, uint8_t chunk_size) : filename(filename),
        input(filename, std::ios::binary), chunk_size(chunk_size),
        cells_per_byte(8 / chunk_size) {}

    /**
     * Opens an output stream for image '.../image.bmp' as
     * './output_bitmaps/image.bmp.out'
     * 
     * @return `true` on success, `false` otherwise
    */
    bool open_ofstream();

    /**
     * Sorts according to member variable seq,
     * useful when extracting data from images
     */
    auto operator<=>(const bmp::image& rhs) const;

    /**
     * Moves reading position of the input stream
     * to data offset.
     */
    void set_data_start();

    std::size_t byte_capacity() const;
};

const int smaller_header_size = 14;

class chunker {
public:
    chunker(std::span<uint8_t> data, uint8_t chunk_size,
        bool is_get = true);
    bool get_chunk(uint8_t& chunk);
    bool send_chunk(uint8_t chunk);

private:
    void split_byte(uint8_t byte);
    void merge_chunks();

    std::span<uint8_t> data;
    std::size_t data_index{0};

    /* how many bits of byte will store hidden data */
    uint8_t chunk_size;
    /* contains the splitted byte in multiple bytes */
    std::vector<uint8_t> chunks;
    std::size_t chunks_index;

    uint8_t mask;
};

constexpr int buffer_size = 256;

class image_buffer {
public:
    image_buffer(bmp::image &im, uint8_t chunk_size);
    bool hide_chunk(uint8_t chunk);
    bool extract_chunk(uint8_t& chunk);
    void change_chunk_size(uint8_t chunk_size);
    void copy_rest();

private:
    bool read();
    bool write_and_read();

    std::array<char, buffer_size> buffer;
    std::size_t index{0};
    std::size_t loaded{0};
    bmp::image &im;

    uint8_t mask;
    uint8_t erase_mask;

    /* bitmap padding */
    /* current row position in the image while processing */
    uint32_t x{0};
    std::size_t skip{0};
};

/**
 * Reads, checks and stores relevant information about bmp image into image
 * struct.
 * 
 * @param im reference to image struct, where information is stored
 * 
 * @return `true` on success, `false` otherwise
 * 
 * @note image struct `im` must have opened ifstream `input` (this should be
 * guaranteed by the image constructor) currently pointing at the beginning
 * of the file. After the call, ifstream `input` is pointing at beginning
 * of bmp's data section.
 */
bool load_header(image &im);

}

#endif  // BITMAP_H
