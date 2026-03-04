#ifndef CHUNKER_H
#define CHUNKER_H

#include <cstdint>
#include <vector>
#include <span>

/**
 * @brief Returns a bitmask for the given chunk size, which can be used to
 * split bytes into chunks and merge them back together.
 */
uint8_t get_mask(uint8_t chunk_size);

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

#endif  // CHUNKER_H
