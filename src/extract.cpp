#include <vector>
#include <ranges>
#include <cassert>

#include "../include/bitmap.h"
#include "../include/extract.h"
#include "../include/info_structs.h"

static unsigned char chunks_to_byte(splitting_info &s)
{
    unsigned char rv = 0;
    for (auto chunk : std::ranges::reverse_view(s.chunks)) {
        rv <<= s.split_size;
        rv |= chunk;
    }
    return rv;
}

static void extract_chunks(splitting_info &s, const bmp::image &im,
    char *buffer, padding_info &p)
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
        s.chunks[i] = *(buffer + i) & s.mask;
    }
}

static unsigned char extract_to_byte(splitting_info &s, const bmp::image &im,
    char *buffer, padding_info &p)
{
    extract_chunks(s, im, buffer, p);
    return chunks_to_byte(s);
}

extracted read_hidden_header(bmp::image &im, padding_info &p)
{
    splitting_info s(2);
    char buffer[256];
    extracted rv;

    std::size_t hm_size_in_bytes = HIDDEN_METADATA_SIZE * s.chunks.size();
    if (!im.in.read(buffer, hm_size_in_bytes)) {
        rv.error_message = "could not read the hidden header";
        return rv;
    }
    if (extract_to_byte(s, im, buffer, p) != 'S') {
        rv.error_message = "invalid magic number";
        return rv;
    }
    if (extract_to_byte(s, im, buffer + s.chunks.size(), p) != 'H') {
        rv.error_message = "invalid magic number";
        return rv;
    }
    im.id = extract_to_byte(s, im, buffer + 2 * s.chunks.size(), p);
    im.seq = extract_to_byte(s, im, buffer + 3 * s.chunks.size(), p);

    for (std::size_t i = 0; i < 4; ++i) {
        rv.data_size |= extract_to_byte(s, im,
                            buffer + (4 + i) * s.chunks.size(), p) << i * 8;
    }
    return rv;
}

bool extract_data(bmp::image &im, std::vector<unsigned char> &data,
    std::size_t data_index, std::size_t data_end_index, padding_info &p)
{
    splitting_info s(2);
    char buffer[256];

    std::size_t bytes = 0;
    while (data_index < data_end_index &&
           (bytes = im.in.read(buffer, sizeof buffer).gcount()) > 0) {

        for (std::size_t i = 0;
             i + s.chunks.size() - 1 < bytes; i += s.chunks.size()) {
            if (data_index >= data_end_index)
                return true;

            data[data_index++] = extract_to_byte(s, im, buffer + i, p);
        }
        im.in.seekg(-(bytes % s.chunks.size()), std::ios::cur);
    }
    return data_index == data_end_index;
}
