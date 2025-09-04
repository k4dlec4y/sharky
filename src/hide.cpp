#include <vector>
#include <cassert>
#include <iostream>
#include <cstring>

#include "../include/bitmap.h"
#include "../include/hide.h"

static void split_byte(std::vector<unsigned char> &chunks, unsigned char ch,
    unsigned char split_size, unsigned char mask)
{
    assert(split_size <= 8);

    for (auto it = chunks.begin(); it != chunks.end(); ++it) {
        *it = ch & mask;
        ch >>= split_size;
    }
}

static void extract_chunks(bmp::image &im, std::vector<unsigned char> &chunks,
    std::size_t &im_i, unsigned char erase_mask, int32_t &x)
{
    for (auto chunk : chunks) {
        /* padding */
        if (x >= im.width * im.channel_count) {
            im_i += im.padding;
            x = -1;
        }
        ++x;

        std::cout << +im.img_data[im_i] << " - ";
        im.img_data[im_i] &= erase_mask;
        im.img_data[im_i++] |= chunk;
        std::cout << +im.img_data[im_i - 1] << std::endl;
    }
}

bool hide_data(bmp::image &im, std::string_view data,
    std::size_t &data_index, unsigned char id, unsigned char seq)
{
    assert(im.byte_capacity > HIDDEN_METADATA_SIZE);
    const unsigned char split_size = 2;

    std::size_t chunk_count = 8 / split_size + (8 % split_size != 0);
    std::vector<unsigned char> chunks(chunk_count);
    unsigned char mask = split_size < 8 ? (1 << split_size) - 1 : 0xffu;
    unsigned char erase_mask = ~mask;

    std::size_t im_i = 0;
    int32_t x = 0;

    split_byte(chunks, 'S', split_size, mask);
    extract_chunks(im, chunks, im_i, erase_mask, x);
    split_byte(chunks, 'H', split_size, mask);
    extract_chunks(im, chunks, im_i, erase_mask, x);

    split_byte(chunks, id, split_size, mask);
    extract_chunks(im, chunks, im_i, erase_mask, x);
    split_byte(chunks, seq, split_size, mask);
    extract_chunks(im, chunks, im_i, erase_mask, x);

    std::size_t data_index_origin = data_index;
    im_i += chunk_count * 4;
    for (; data_index < data.size() && im_i < im.byte_capacity; ++data_index) {
        split_byte(chunks, data[data_index], split_size, mask);
        extract_chunks(im, chunks, im_i, erase_mask, x);
    }

    std::size_t data_size = data_index - data_index_origin;
    im_i = chunk_count * 4;
    for (std::size_t _ = 0; _ < 4; ++_) {
        unsigned char chunk = data_size & 0xffu;
        split_byte(chunks, chunk, split_size, mask);
        extract_chunks(im, chunks, im_i, erase_mask, x);
        data_size >>= 8;
    }
    return data_index != data.size();
}
