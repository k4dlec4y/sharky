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

/**
 * @brief Struct representing a bmp image, containing all relevant information
 * about the image, as well as input and output streams for reading and writing
 * the image file.
 */
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
     * @brief This constructor does not open the input nor output file, it only
     * initializes the filename and chunk_size members. The input/output files
     * should be opened by calling `assign_input`/`assign_output` methods.
     */
    image(const std::string &filename, uint8_t chunk_size);

    /**
     * @brief Opens the input stream for the image using the filename
     * member variable.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_input();

    /**
     * @brief Assigns the input stream for the image. The input stream
     * should be already opened and ready to read.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_input(std::unique_ptr<std::istream> input);

    /**
     * @brief Trims the filename and returns path to output file,
     * which is "sharky/bitmaps_out/" + trimmed filename
     * 
     * @return path to output file
     */
    std::string get_output_path();

    /**
     * @brief Writes the header of the bmp file to the output file.
     * This should be called after opening the output file and before hiding
     * any data, so that the output file is a valid bmp file.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool write_header_to_output();

    /**
     * @brief Opens the output stream for the image using the get_output_path
     * method.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_output();

    /**
     * @brief Assigns the output stream for the image. The output stream
     * should be already opened and ready to write.
     * 
     * @return `true` on success, `false` otherwise
     */
    bool assign_output(std::unique_ptr<std::ostream> output);

    /**
     * @brief Image comparison operator, compares images by their sequence
     * number. This is useful when extracting data from images.
     */
    auto operator<=>(const bmp::image &rhs) const;

    /**
     * @brief Moves reading position of the input stream to data offset.
     */
    void set_data_start();

    /**
     * @brief Returns how many bytes can be hidden into the image in total,
     * excluding the size of metadata. This is calculated
     * as capacity / cells_per_byte.
     * 
     * @return byte capacity of the image for hidden data, excluding metadata
     */
    std::size_t byte_capacity() const;
};

/**
 * @brief Class used for splitting bytes of data to into smaller chunks,
 * and merging them back together when extracting.
 */
class chunker {
public:
    /**
     * @param data data to be split into chunks or where merged chunks
     * will be stored
     * @param chunk_size size of chunk in bits,
     * how many bits of byte will store hidden data
     * @param is_splitting `true` if the chunker will be used for splitting
     * bytes into chunks, `false` if the chunker will be used for merging
     * bytes from chunks
     * @note `is_splitting` does not guarantee that the chunker can be used
     * for one purpose only, it only initializes inner state
     */
    chunker(std::span<uint8_t> data, uint8_t chunk_size,
        bool is_splitting = true);

    /**
     * @brief Used for splitting. Gets the next chunk of data. If the end
     * of data is reached, it returns `false`, otherwise it returns `true`
     * and stores the chunk in the provided reference.
     */
    bool get_chunk(uint8_t &chunk);

    /**
     * @brief Used for merging. Sends the next chunk of data. If it was
     * the last chunk and there is no more space in the data, it returns
     * `false`, otherwise it returns `true` and stores the chunk in the data. 
     */
    bool send_chunk(uint8_t chunk);

private:
    void split_byte(uint8_t byte);
    void merge_chunks();

    std::span<uint8_t> data;
    std::size_t data_index{0};

    uint8_t chunk_size;
    /* contains the splitted byte in multiple bytes */
    std::vector<uint8_t> chunks;
    std::size_t chunks_index;

    uint8_t mask;
};

const std::size_t BUFFER_SIZE = 4096;

/**
 * @brief Class used for hiding and extracting data from bmp images. It contains
 * a buffer for reading and writing data to the image file, as well as methods
 * for hiding and extracting chunks of data.
 */
class image_buffer {
public:
    /**
     * @param im image to be used for hiding/extracting data
     * @param chunk_size size of chunk in bits, how many bits of byte
     * will store hidden data
     */
    image_buffer(bmp::image &im, uint8_t chunk_size);

    /**
     * @brief Hides the provided chunk of data into the image. It returns `true`
     * on success, and `false` if there is no more space in the image to hide
     * the chunk, or if there was an error while reading/writing the image file.
     */
    bool hide_chunk(uint8_t chunk);

    /**
     * @brief Extracts the next chunk of data from the image to the provided
     * reference. It returns `true` on success, and `false` if there is no more
     * data to extract, or if there was an error while reading the image file.
     */
    bool extract_chunk(uint8_t &chunk);

    /**
     * @brief Changes the chunk size of the image buffer. This should be called
     * when the chunk size of the image is determined after extracting metadata.
     */
    void change_chunk_size(uint8_t chunk_size);

    /**
     * @brief Copies the rest of the image data from the input file
     * to the output file without hiding any data. This should be called
     * after hiding all the data, to ensure that the output file is
     * a valid bmp file with all the original data, except for the hidden data.
     */
    void copy_rest();

private:
    bool read();
    bool write_and_read();

    /**
     * @brief Moves index to the next position where data can
     * be hidden/extracted, while skipping padding bytes. This method
     * is called automatically by `hide_chunk` and `extract_chunk` methods,
     * and it uses the provided function to read more data into the buffer
     * if needed. It returns `true` if the index was successfully moved
     * to the next position, `false` otherwise.
     */
    bool move_index(std::function<bool(void)> read_or_writeread);

    std::array<char, BUFFER_SIZE> buffer;
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
 * @param err stream where error messages will be written, default is std::cerr
 * 
 * @return `true` on success, `false` otherwise
 * 
 * @note `im` should have its input stream already ready to read before calling
 * this function, and after calling this function, the input stream will
 * be positioned at the beginning of the image pixel data.
 */
bool load_header(image &im, std::ostream &err = std::cerr);

}

#endif  // BITMAP_H
