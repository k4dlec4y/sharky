#ifndef INFO_STRUCTS_H
#define INFO_STRUCTS_H

#include <vector>
#include <cstdint>

struct splitting_info {
    /* how many bits of byte will store hidden data */
    uint8_t split_size;
    /* contains the splitted byte in multiple bytes */
    std::vector<uint8_t> chunks;
    uint8_t mask;
    uint8_t erase_mask;

    splitting_info(uint8_t split_size) :
        split_size(split_size),
        chunks((8 + chunk_size - 1) / chunk_size),
        mask(split_size < 8 ? (1 << split_size) - 1 : 0xffu) {
        erase_mask = ~mask;
    }
};

struct padding_info {
    /* current row position in the image while processing */
    int32_t x{0};
    /* if > 0, we shouldn't hide/extract data */
    std::size_t skip{0};
};

#endif // INFO_STRUCTS_H
