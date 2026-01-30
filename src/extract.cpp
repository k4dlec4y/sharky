#include <vector>
#include <span>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <iostream>

#include "../include/bitmap.h"
#include "../include/extract.h"

static bool extract_bytes(
    bmp::image_buffer& buffer,
    bmp::chunker& chunker,
    auto size
) {
    uint8_t chunk;
    for (auto _ = 0u; _ < size * (8 / chunk_size); ++_) {
        if (!buffer.extract_chunk(chunk) || !chunker.send_chunk(chunk)) {
            return false;
        }
    }
    return true;
}

bool extract_hidden_metadata(
    bmp::image& im,
    bmp::image_buffer& buffer
) {
    std::vector<uint8_t> data(hidden_metadata_size);
    bmp::chunker chunker{std::span(data.data(), data.size()), chunk_size, false};

    if (!extract_bytes(buffer, chunker, hidden_metadata_size)) {
        std::cerr << "image " << im.filename
                  << " run out of bytes too early!\n";
        return false;
    }

    if (data[0] != 'S' || data[1] != 'H') {
        std::cerr << "image " << im.filename
                  << " has invalid sharky magic number! ("
                  << int(data[0]) << ", " << int(data[1]) << ")\n";
        return false;
    }

    im.id = data[2];
    im.seq = data[3];

    im.hidden_data_size = 0u;
    for (std::size_t i = 0; i < 4; ++i)
        im.hidden_data_size |= data[4 + i] << (i * 8);
    return true;
}

bool extract_data(
    bmp::image& im,
    bmp::image_buffer& buffer,
    std::span<uint8_t> data
) {
    bmp::chunker chunker {data, chunk_size, false};
    if (!extract_bytes(buffer, chunker, data.size())) {
        std::cerr << "image " << im.filename
                  << " run out of bytes too early!\n";
        return false;
    }
    return true;
}

int extract(
    std::vector<bmp::image>& images,
    std::vector<uint8_t>& data
) {
    std::vector<bmp::image_buffer> buffers{};
    auto data_size = 0;

    for (auto i = 0u; i < images.size(); ++i) {
        buffers.emplace_back(images[i], chunk_size);
        if (!extract_hidden_metadata(images[i], buffers[i]))
            return 1;
        data_size += images[i].hidden_data_size;
    }

    std::vector<size_t> indx(images.size());
    std::iota(indx.begin(), indx.end(), 0);
    std::ranges::sort(indx, {},
                      [&](size_t i) { return images[i].seq; });

    for (auto i = 0u; i < images.size(); ++i) {
        auto& im = images[indx[i]];
        if (im.seq != i) {
            std::cerr << "image " << im.filename << " has invalid number,"
                      << "it should be " << i << "but its seq number is "
                      << im.seq;
            return 1;
        }
    }

    data.resize(data_size);
    auto data_index = 0;
    for (auto i = 0u; i < images.size(); ++i) {
        auto j = indx[i];
        auto n = images[j].hidden_data_size;
        if (!extract_data(images[j], buffers[j],
                          std::span(data.data() + data_index, n)))
            return 1;
        data_index += n;
    }
    return 0;
}
