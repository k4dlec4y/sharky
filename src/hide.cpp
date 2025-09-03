#include <vector>
#include <cassert>
#include <iostream>

#include "../include/bitmap.h"
#include "../include/hide.h"

static std::size_t min(std::size_t a, std::size_t b)
{
    return a < b ? a : b;
}

static void split_byte(std::vector<unsigned char> &array, unsigned char ch,
    std::size_t split_size, std::size_t chunk_count, std::size_t mask)
{
    assert(split_size <= 8);

    for (std::size_t i = 0; i < chunk_count; ++i) {
        array[i] = ch & mask;
        ch >>= split_size;
    }
}

void hide_data(bmp::image &im, unsigned char *data, std::size_t data_size,
    std::size_t split_size)
{
    assert(data_size <= im.byte_capacity);

    std::size_t chunk_count = 8 / split_size + (8 % split_size != 0);
    std::vector<unsigned char> chunks(chunk_count);
    unsigned char mask = split_size < 8 ? (1 << split_size) - 1 : 255u;
    unsigned char erase_mask = ~mask;

    std::size_t im_i = 0;
    for (std::size_t i = 0; i < min(im.byte_capacity, data_size); ++i) {
        split_byte(chunks, data[i], split_size, chunk_count, mask);

        for (std::size_t j = 0; j < chunk_count; ++j) {
            std::cout << +im.img_data[im_i] << " - ";
            im.img_data[im_i] &= erase_mask;
            im.img_data[im_i++] |= chunks[j];
            std::cout << +im.img_data[im_i - 1] << std::endl;
        }
    }
}
