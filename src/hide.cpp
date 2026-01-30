#include <span>
#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>

#include "../include/bitmap.h"
#include "../include/hide.h"

bool hide_data(
    bmp::image &im,
    std::span<uint8_t> to_hide,
    uint8_t id,
    uint8_t seq
) {
    /* this should never happen, handled in bitmap files */
    if (im.byte_capacity < hidden_metadata_size + to_hide.size())
        return false;

    im.open_ofstream();
    im.output.write(reinterpret_cast<char *>(im.header.data()), im.header.size());

    bmp::image_buffer buffer{im, chunk_size};

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

    uint8_t chunk;
    bmp::chunker metadata_chnkr{std::span(metadata.data(), metadata.size()), chunk_size};
    while (metadata_chnkr.get_chunk(chunk)) {
        if (!buffer.hide_chunk(chunk)) {
            std::cerr << "image file " << im.filename
                      << " is smaller than expected or there is not enough space "
                      << "for altered image on disk (io error)\n";
            return false;
        }
    }
 
    bmp::chunker data_chnkr{to_hide, chunk_size};
    while (data_chnkr.get_chunk(chunk)) {
        if (!buffer.hide_chunk(chunk)) {
            std::cerr << "image file " << im.filename
                      << " is smaller than expected or there is not enough space "
                      << "for altered image on disk (io error)\n";
            return false;
        }
    }

    buffer.copy_rest();

    im.output.close();
    return true;
}

int hide(std::vector<bmp::image> &images, std::string data_path) {

    auto data_size = std::filesystem::file_size(data_path);
    std::ifstream data_ifstream{data_path, std::ios::binary};

    std::vector<uint8_t> data(data_size);
    data_ifstream.read(reinterpret_cast<char*>(data.data()), data_size);
    std::span span(data);

    /* will become randomly generated later */
    uint8_t id = 137u;

    uint8_t seq = 0u;
    auto data_index = 0ul;

    for (; data_index < data_size && seq < images.size(); ++seq) {
        auto capacity = images[seq].byte_capacity - hidden_metadata_size;
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
