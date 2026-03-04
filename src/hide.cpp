#include <span>
#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <filesystem>

#include "../include/configuration.h"
#include "../include/bitmap.h"
#include "../include/hide.h"

static bool hide_bytes(
    bmp::chunker &chnkr,
    bmp::image_buffer &buffer,
    std::string_view image_filename
) {
    uint8_t chunk;
    while (chnkr.get_chunk(chunk)) {
        if (!buffer.hide_chunk(chunk)) {
            std::cerr << "image file " << image_filename << " is smaller "
                      << "than expected or there is not enough space "
                      << "for altered image on disk\n";
            return false;
        }
    }
    return true;
}

bool hide_data(
    bmp::image &im,
    std::span<uint8_t> to_hide,
    uint8_t id,
    uint8_t seq
) {
    bmp::image_buffer buffer{im, MD_CHUNK_SIZE};

    std::vector<uint8_t> metadata{};
    /* magic number for sharky images */
    metadata.emplace_back(static_cast<uint8_t>('S'));
    metadata.emplace_back(static_cast<uint8_t>('H'));

    metadata.emplace_back(id);
    metadata.emplace_back(seq);

    auto data_size = static_cast<uint32_t>(to_hide.size());
    for (auto _ = 0u; _ < sizeof(uint32_t); ++_) {
        metadata.emplace_back(static_cast<uint8_t>(data_size & 0xffu));
        data_size >>= 8;
    }
    metadata.emplace_back(im.chunk_size);

    bmp::chunker metadata_chnkr{
        std::span(metadata.data(), metadata.size()), MD_CHUNK_SIZE};

    if (!hide_bytes(metadata_chnkr, buffer, im.filename))
        return false;

    buffer.change_chunk_size(im.chunk_size);
    bmp::chunker data_chnkr{to_hide, im.chunk_size};
    if (!hide_bytes(data_chnkr, buffer, im.filename))
        return false;

    buffer.copy_rest();
    return true;
}

static uint8_t generate_id() {
    std::default_random_engine e(std::random_device{}());

    std::uniform_int_distribution<int> dist(1, 255);
    return static_cast<uint8_t>(dist(e));
}

int hide(std::vector<bmp::image> &images, std::istream &data_in) {

    std::size_t data_size = data_in.seekg(0, std::ios::end).tellg();
    data_in.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(data_size);
    data_in.read(reinterpret_cast<char*>(data.data()), data_size);
    std::span span(data);

    uint8_t id = generate_id();

    uint8_t seq = 0u;
    auto data_index = 0ul;

    for (; data_index < data_size && seq < images.size(); ++seq) {
        auto capacity = images[seq].byte_capacity();
        std::cout << "image " << images[seq].filename
                  << " was opened with "
                  << capacity << " byte capacity\n";
        auto sspan_size = std::min(capacity, data_size - data_index);
        std::span data_part = span.subspan(data_index, sspan_size);

        if (!hide_data(images[seq], data_part, id, seq))
            return 2;
        data_index += capacity;
    }
    for (; seq < images.size(); ++seq) {
        std::cout << "image " << images[seq].filename
                  << " was not neccessary to hide data\n";
    }

    if (data_index < data_size) {
        std::cerr << "only first " << data_index << " bytes were hidden, "
                  << "please use more or larger images ("
                  << data_size << " capacity is needed)\n";
        return 1;
    }
    return 0;
}
