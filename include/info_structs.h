#ifndef INFO_STRUCTS_H
#define INFO_STRUCTS_H

#include <vector>
#include <cstdint>

struct splitting_info {
    unsigned char split_size;
    std::vector<unsigned char> chunks;
    unsigned char mask;
    unsigned char erase_mask;

    splitting_info(unsigned char split_size) :
        split_size(split_size),
        chunks(8 / split_size + (8 % split_size != 0)),
        mask(split_size < 8 ? (1 << split_size) - 1 : 0xffu) {
        erase_mask = ~mask;
    }
};

struct padding_info {
    int32_t x{0};
    std::size_t skip{0};
};

#endif // INFO_STRUCTS_H
