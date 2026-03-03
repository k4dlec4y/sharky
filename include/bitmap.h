#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <vector>
#include <span>
#include <fstream>
#include <functional>
#include <memory>

#include "../include/configuration.h"

namespace bmp {

struct image {
    std::string filename;
    std::unique_ptr<std::istream> input{nullptr};
    std::unique_ptr<std::ostream> output{nullptr};
    uint32_t width{0};
    uint32_t height{0};
    /* 3 for BGR, 4 for BGRA*/
    uint16_t channel_count{0};

    uint32_t data_offset{0};
    /* how many bytes of the image can be used for hidding data */
    /* this already excludes size for hidden metadata */
    std::size_t capacity{0};
    uint8_t chunk_size;
    uint8_t cells_per_byte;
    /* this excludes the size of metadata! */
    std::size_t hidden_data_size{0};
    /* bitmap padding */
    int padding{0};

    /* used for extraction */
    uint8_t id{0};
    uint8_t seq{0};

    std::vector<uint8_t> header{};

    /**
     * This constructor does not open the input nor output file, it only
     * initializes the filename and chunk_size members. The input/output files
     * should be opened by calling `assign_input`/`assign_output` methods.
     */
    image(const std::string &filename, uint8_t chunk_size);

    /**
     * Opens the input stream for the image using the filename member variable.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_input();

    /**
     * Assigns the input stream for the image. The input stream should be already
     * opened and ready to read.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_input(std::unique_ptr<std::istream> input);

    /**
     * Trims the filename and returns path to output file,
     * which is "sharky/bitmaps_out/" + trimmed filename
     * 
     * @return path to output file
     */
    std::string get_output_path();

    /**
     * Writes the header of the bmp file to the output file.
     * This should be called after opening the output file and before hiding
     * any data, so that the output file is a valid bmp file.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool write_header_to_output();

    /* Opens the output stream for the image using the get_output_path method.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_output();

    /**
     * Assigns the output stream for the image. The output stream should be already
     * opened and ready to write.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_output(std::unique_ptr<std::ostream> output);

    /**
     * Image comparison operator, compares images by their sequence number.
     * This is useful when extracting data from images.
     */
    auto operator<=>(const bmp::image &rhs) const;

    /**
     * Moves reading position of the input stream
     * to data offset.
     */
    void set_data_start();

    /**
     * Returns how many bytes can be hidden into the image in total,
     * excluding the size of metadata. This is calculated as capacity / cells_per_byte.
     * 
     * @return byte capacity of the image for hidden data, excluding metadata
     */
    std::size_t byte_capacity() const;
};

const int smaller_header_size = 14;

class chunker {
public:
    chunker(std::span<uint8_t> data, uint8_t chunk_size,
        bool is_get = true);
    bool get_chunk(uint8_t &chunk);
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

constexpr int buffer_size = 4096;

class image_buffer {
public:
    /**
     * @param im reference to image struct, where information is stored
     * @param chunk_size how many bits of byte will store hidden data, used
     * to calculate masks and buffer size
     */
    image_buffer(bmp::image &im, uint8_t chunk_size);
    bool hide_chunk(uint8_t chunk);
    bool extract_chunk(uint8_t &chunk);
    void change_chunk_size(uint8_t chunk_size);
    void copy_rest();

private:
    bool read();
    bool write_and_read();

    /**
     * Moves index to the next position where data can be hidden/extracted, while
     * skipping padding bytes. If the end of buffer is reached, it will call
     * the provided function to write the buffer and read the next chunk of data
     * into the buffer. The provided function should return `true` if the next
     * chunk of data was successfully read into the buffer, and `false` if
     * there is no more data to read.
     */
    bool move_index(std::function<bool(void)> read_or_writeread);

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
 * @param error_stream stream where error messages will be written, default is std::cerr
 * 
 * @return `true` on success, `false` otherwise
 * 
 * @note image struct `im` must have opened ifstream `input` (this should be
 * guaranteed by the image constructor) currently pointing at the beginning
 * of the file. After the call, ifstream `input` is pointing at beginning
 * of bmp's data section.
 */
bool load_header(image &im, std::ostream &error_stream = std::cerr);

}

#endif  // BITMAP_H
