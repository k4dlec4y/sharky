#include <vector>
#include <cassert>
#include <iostream>
#include <cstring>

#include "../include/bitmap.h"
#include "../include/hide.h"

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

static void split_byte(splitting_info &s, unsigned char to_hide)
{
    assert(s.split_size <= 8);

    for (auto it = s.chunks.begin(); it != s.chunks.end(); ++it) {
        *it = to_hide & s.mask;
        to_hide >>= s.split_size;
    }
}

static void hide_chunks(splitting_info &s, bmp::image &im, char *buffer,
    padding_info &p)
{
    for (std::size_t i = 0; i < s.chunks.size(); ++i) {
        /* padding */
        if (p.x >= im.width * im.channel_count) {
            p.skip = im.padding;
            p.x = -1;
        }
        ++p.x;

        if (p.skip > 0) {
            --p.skip;
            continue;
        }
        *(buffer + i) &= s.erase_mask;
        *(buffer + i) |= s.chunks[i];
    }
}

static void split_and_hide(splitting_info &s, bmp::image &im, char *byte,
    padding_info &p, unsigned char to_hide)
{
    split_byte(s, to_hide);
    hide_chunks(s, im, byte, p);
}

void hide_data(bmp::image &im, std::string_view data, unsigned char id,
    unsigned char seq)
{
    assert(im.byte_capacity >= HIDDEN_METADATA_SIZE + data.size());

    splitting_info s(2);
    padding_info p;
    char buffer[256];
    std::size_t data_index = 0;

    im.open_ofstream();
    im.out.write(reinterpret_cast<char *>(im.header.data()), im.header.size());

    std::size_t hm_size_in_bytes = HIDDEN_METADATA_SIZE * s.chunks.size();
    auto bytes = im.in.read(buffer, hm_size_in_bytes).gcount();

    split_and_hide(s, im, buffer, p, 'S');
    split_and_hide(s, im, buffer + s.chunks.size(), p, 'H');

    split_and_hide(s, im, buffer + 2 * s.chunks.size(), p, id);
    split_and_hide(s, im, buffer + 3 * s.chunks.size(), p, seq);

    std::size_t data_size = data.size();
    for (std::size_t i = 4 * s.chunks.size(); i < 8 * s.chunks.size();
         i += s.chunks.size()) {
        split_and_hide(s, im, buffer + i, p, data_size & 0xffu);
        data_size >>= 8;
    }
    im.out.write(buffer, hm_size_in_bytes);

    while ((bytes = im.in.read(buffer, sizeof buffer).gcount()) > 0) {
        for (std::size_t i = 0;
             i + s.chunks.size() - 1 < static_cast<std::size_t>(bytes);
             i += s.chunks.size()) {
            if (data_index >= data.size()) {
                im.out.write(buffer, bytes);
                goto copy;
            }
            split_and_hide(s, im, buffer + i, p, data[data_index++]);
        }
        im.out.write(buffer, bytes);
        im.in.seekg(-(bytes % s.chunks.size()), std::ios::cur);
    }
copy:
    while ((bytes = im.in.read(buffer, sizeof buffer).gcount()) > 0) {
        im.out.write(buffer, bytes);
    }
    im.out.close();
}
