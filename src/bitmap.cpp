#include <array>
#include <span>
#include <ranges>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "../include/bitmap.h"

namespace bmp {

static uint8_t get_mask(uint8_t chunk_size) {
    return chunk_size < 8
           ? static_cast<uint8_t>((1u << chunk_size) - 1)
           : 0xffu;
}

class chunker {
    std::span<uint8_t> data;
    std::size_t data_index{0};

    uint8_t chunk_size;
    std::vector<uint8_t> chunks;
    std::size_t chunks_index;

    uint8_t mask;

public:
    chunker(std::span<uint8_t> data, uint8_t chunk_size) :
        data(data),
        chunk_size(chunk_size),
        chunks((8 + chunk_size - 1) / chunk_size),
        chunks_index((8 + chunk_size - 1) / chunk_size),
        mask(get_mask(chunk_size)) {}

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

    bool send_chunk(uint8_t chunk) {
        if (chunks_index >= chunks.size()) {
            if (data_index >= data.size())
                return false;
            merge_chunks();
            chunks_index = 0;
        }
        chunks[chunks_index++] = chunk;
        return true;
    }

private:
    void split_byte(uint8_t byte) {
        for (auto& chunk : chunks) {
            chunk = byte & mask;
            byte >>= chunk_size;
        }
    }

    void merge_chunks() {
        uint8_t merged = 0;
        for (auto& chunk : std::ranges::reverse_view(chunks)) {
            merged <<= chunk_size;
            merged |= chunk;
        }
        data[data_index++] = merged;
    }
};

constexpr int buffer_size = 256;

class image_buffer {
    std::array<char, buffer_size> buffer;
    std::size_t index{0};
    std::size_t loaded{0};
    bmp::image &im;

    uint8_t mask;
    uint8_t erase_mask;

    int32_t x{0};
    std::size_t skip{0};

public:
    image_buffer(bmp::image &im, uint8_t chunk_size) : im(im),
        mask(get_mask(chunk_size)) {
        im.set_data_start();
        erase_mask = ~mask;
    }

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
        buffer[index++] |= chunk;
        ++x;
        return true;
    }

    bool extract_chunk(uint8_t& chunk) {
        while (true) {
            if (index >= loaded) {
                if (!read())
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

        chunk = buffer[index++] & mask;
        ++x;
        return true;
    }

    void copy_rest() {
        while (write_and_read()) {}
    }

private:
    bool read() {
        buffer.fill(static_cast<char>(0));
        im.input.read(buffer.data(), buffer_size);
        loaded = im.input.gcount();
        index = 0;
        return loaded > 0;
    }

    bool write_and_read() {
        if (loaded > 0)
            im.output.write(buffer.data(), loaded);
        return read();
    }
};

bool bmp::image::open_ofstream() {
    using namespace std::string_literals;

    auto basename_index = filename.rfind('/');
    if (basename_index == std::string::npos)
        basename_index = 0;
    else
        ++basename_index;
    auto basename = filename.substr(basename_index);

    std::filesystem::create_directories("output_bitmaps");

    output.open("output_bitmaps/"s + basename + ".out"s,
        std::ios::binary | std::ios::trunc);
    return output.good();
}

auto bmp::image::operator<=>(const bmp::image& rhs) const {
    return this->seq <=> rhs.seq;
}

void bmp::image::set_data_start() {
    this->input.seekg(this->data_offset, std::ios::beg);
}

static int16_t to_int16(const uint8_t *data) {
    return static_cast<int16_t>(data[0]) |
           (static_cast<int16_t>(data[1]) << 8);
}

static int32_t to_int32(const uint8_t *data) {
    return static_cast<int32_t>(data[0]) |
           (static_cast<int32_t>(data[1]) << 8) |
           (static_cast<int32_t>(data[2]) << 16) |
           (static_cast<int32_t>(data[3]) << 24);
}

static int count_padding(int width, int channels) {
    return (4 - (width * channels) % 4) % 4;
}

bool load_header(image &im, std::size_t cells_per_byte) {

    auto file_size = std::filesystem::file_size(im.filename);
    im.header.resize(smaller_header_size);

    if (file_size < smaller_header_size) {
        std::cerr << "file " << im.filename << " is too small to be bmp\n";
        return false;
    }
    if (!im.input.read(reinterpret_cast<char *>(im.header.data()),
                       smaller_header_size)) {
        std::cerr << "file " << im.filename << " could not be read\n";
        return false;
    }
    if (im.header[0] != 'B' || im.header[1] != 'M') {
        std::cerr << "file " << im.filename << " has invalid magic number to be"
            " bmp file: " << im.header[0] << im.header[1] << '\n';
        return false;
    }
    if (to_int32(im.header.data() + 2) != file_size) {
        std::cerr << "file " << im.filename << " - the actual size and size "
            "in bmp header does not match\n";
        return false;
    }
    im.data_offset = to_int32(im.header.data() + 10);
    im.header.resize(im.data_offset);
    if (!im.input.read(reinterpret_cast<char *>(im.header.data()) + smaller_header_size,
                       im.data_offset - smaller_header_size)) {
        std::cerr << "file " << im.filename << " could not be read\n";
        return false;
    }
    im.width = to_int32(im.header.data() + 18);
    im.height = to_int32(im.header.data() + 22);

    int16_t bit_count = to_int16(im.header.data() + 28);
    if (bit_count != 24 && bit_count != 32) {
        std::cerr << "file " << im.filename << " has invalid bit count "
            "for single pixel, use 24/32\n";
        return false;
    }
    im.channel_count = bit_count / 8;
    im.padding = count_padding(im.width, im.channel_count);

    im.byte_capacity = static_cast<std::size_t>(im.width) *
        im.channel_count * im.height / cells_per_byte;
    if (im.byte_capacity <= hidden_metadata_size) {
        std::cerr << "file " << im.filename << " is too small to hide data\n";
        return false;
    }
    int32_t compression = to_int32(im.header.data() + 30);
    if (compression) {
        std::cerr << "file " << im.filename << " is compressed\n";
        return false;
    }
    return true;
}

}
