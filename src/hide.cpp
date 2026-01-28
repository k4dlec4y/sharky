#include <vector>
#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <filesystem>

#include "../include/bitmap.h"
#include "../include/hide.h"
#include "../include/info_structs.h"

/* will become configurable at cmd later */
const uint8_t chunk_size = 2;

static uint8_t get_mask(uint8_t chunk_size) {
    return chunk_size < 8
           ? static_cast<uint8_t>((1u << chunk_size) - 1)
           : 0xffu;
}

class chunker {
    std::string_view data;
    std::size_t data_index{0};

    /* how many bits of byte will store hidden data */
    uint8_t chunk_size;
    /* contains the splitted byte in multiple bytes */
    std::vector<uint8_t> chunks;
    std::size_t chunks_index;

    uint8_t mask;

public:
    chunker(std::string_view data, uint8_t chunk_size) :
        data(data),
        chunk_size(chunk_size),
        chunks((8 + chunk_size - 1) / chunk_size),
        chunks_index(8 / chunk_size),
        mask(get_mask(chunk_size)) {}

private:
    void split_byte(uint8_t byte) {
        assert(chunk_size <= 8);

        for (auto& chunk : chunks) {
            chunk = byte & mask;
            byte >>= chunk_size;
        }
    }

public:
    bool get_chunk(uint8_t& chunk) {
        if (chunks_index >= chunks.size()) {
            if (data_index >= data.size())
                return false;
            split_byte(data[data_index++]);
            chunks_index = 0;
        }
        chunk = chunks[chunks_index++];
        return true;
    }
};

constexpr int buffer_size = 256;

class image_buffer {
    std::array<char, buffer_size> buffer;
    std::size_t index{0};
    std::size_t loaded{0};
    bmp::image &im;

    uint8_t erase_mask;

    /* for padding */
    /* current row position in the image while processing */
    int32_t x{0};
    std::size_t skip{0};

public:
    image_buffer(bmp::image &im, uint8_t chunk_size) : im(im),
        erase_mask(~get_mask(chunk_size)) {}

private:
    bool write_and_read() {
        if (loaded > 0)
            im.output.write(buffer.data(), loaded);

        buffer.fill(static_cast<char>(0));
        im.input.read(buffer.data(), buffer_size);
        loaded = im.input.gcount();
        index = 0;
        return loaded > 0;
    }

public:
    bool hide_chunk(uint8_t chunk) {
        while (true) {
            if (index >= loaded) {
                if (!write_and_read())
                    return false;
            }
            if (x >= im.width * im.channel_count) {
                skip = im.padding;
                x = 0;
            }
            if (skip == 0)
                break;
            ++index;
            --skip;
        }

        buffer[index] &= erase_mask;
        buffer[index] |= chunk;
        ++index;
        ++x;
        return true;
    }

    void copy_rest() {
        while (write_and_read()) {}
    }
};

bool hide_data(bmp::image &im, std::string_view to_hide, uint8_t id,
    uint8_t seq)
{
    if (im.byte_capacity < hidden_metadata_size + to_hide.size())
        return false;

    im.open_ofstream();
    im.output.write(reinterpret_cast<char *>(im.header.data()), im.header.size());

    image_buffer buffer{im, chunk_size};

    /* magic number for sharky images */
    std::string metadata{"SH"};
    metadata += id;
    metadata += seq;
    auto data_size = static_cast<uint32_t>(to_hide.size());
    for (auto _ = 0; _ < sizeof(uint32_t); ++_) {
        metadata += static_cast<uint8_t>(data_size & 0xffu);
        data_size >>= 8;
    }

    uint8_t chunk;
    chunker metadata_chnkr{metadata, chunk_size};
    while (metadata_chnkr.get_chunk(chunk)) {
        if (!buffer.hide_chunk(chunk)) {
            std::cerr << "unexpected error - image and file size probably doesn't match\n";
            return false;
        }
    }

    chunker data_chnkr{to_hide, chunk_size};
    while (data_chnkr.get_chunk(chunk)) {
        if (!buffer.hide_chunk(chunk)) {
            std::cerr << "unexpected error - image and file size probably doesn't match\n";
            return false;
        }
    }

    buffer.copy_rest();

    im.output.close();
    return true;
}

int hide(std::vector<bmp::image> &images, std::string message_path)
{
    auto data_size = std::filesystem::file_size(message_path);
    std::ifstream message_ifstream{message_path, std::ios::binary};

    std::string message(data_size, '\0');
    message_ifstream.read(message.data(), data_size);
    std::string_view message_sv(message);

    /* will become randomly generated later */
    uint8_t id = 137u;

    uint8_t seq = 0u;
    auto message_index = 0ul;
    for (; message_index < data_size && seq < images.size(); ++seq) {
        auto capacity = images[seq].byte_capacity - hidden_metadata_size;
        if (!hide_data(images[seq], message_sv.substr(message_index, capacity), id, seq))
            return 2;
        message_index += capacity;
    }
    for (; seq < images.size(); ++seq) {
        std::cout << "image " << images[seq].filename << " was not"
            "neccessary to hide data\n";
    }

    if (message_index < data_size) {
        std::cerr << "only first " << message_index << "bytes were hidden, please "
            "use more or larger images\n";
        return 1;
    }
    return 0;
}
