#include <vector>
#include <ranges>
#include <cassert>

#include "../include/bitmap.h"
#include "../include/extract.h"
#include "../include/info_structs.h"

static std::size_t min(std::size_t a, std::size_t b)
{
    return a < b ? a : b;
}

static unsigned char chunks_to_byte(const std::vector<unsigned char> &chunks,
    unsigned char split_size)
{
    unsigned char rv = 0;
    for (auto chunk : std::ranges::reverse_view(chunks)) {
        rv <<= split_size;
        rv |= chunk;
    }
    return rv;
}

static void extract_chunks(bmp::image &im, std::vector<unsigned char> &chunks,
    std::size_t &im_i, unsigned char extract_mask, int32_t &x)
{
    for (auto it = chunks.begin(); it != chunks.end(); ++it) {
        /* padding */
        if (x >= im.width * im.channel_count) {
            im_i += im.padding;
            x = -1;
        }
        ++x;

        *it = im.img_data[im_i++] & extract_mask;
    }
}

extracted extract_data(bmp::image &im, std::vector<unsigned char> &data,
    unsigned char id)
{
    assert(im.byte_capacity > HIDDEN_METADATA_SIZE);
    const unsigned char split_size = 2;

    std::size_t chunk_count = 8 / split_size + (8 % split_size != 0);
    std::vector<unsigned char> chunks(chunk_count);
    unsigned char extract_mask = split_size < 8 ? (1 << split_size) - 1 : 0xffu;

    std::size_t im_i = 0;
    int32_t x = 0;

    extracted rv = {false, ""};

    extract_chunks(im, chunks, im_i, extract_mask, x);
    if (chunks_to_byte(chunks, split_size) != 'S') {
        rv.error_message = "invalid magic number";
        return rv;
    }
    extract_chunks(im, chunks, im_i, extract_mask, x);
    if (chunks_to_byte(chunks, split_size) != 'H') {
        rv.error_message = "invalid magic number";
        return rv;
    }

    extract_chunks(im, chunks, im_i, extract_mask, x);
    if (chunks_to_byte(chunks, split_size) != id) {
        rv.error_message = "image doesn't belong to the group according to ID";
        return rv;
    }
    extract_chunks(im, chunks, im_i, extract_mask, x);
    im.seq = chunks_to_byte(chunks, split_size);

    std::size_t data_size = 0;
    for (std::size_t i = 0; i < 32; i += 8) {
        extract_chunks(im, chunks, im_i, extract_mask, x);
        data_size |= chunks_to_byte(chunks, split_size) << i;
    }

    for (std::size_t _ = 0;
         _ < min(data_size, im.byte_capacity - HIDDEN_METADATA_SIZE); ++_) {
        extract_chunks(im, chunks, im_i, extract_mask, x);
        data.emplace_back(chunks_to_byte(chunks, split_size));
    }

    rv.success = true;
    return rv;
}
