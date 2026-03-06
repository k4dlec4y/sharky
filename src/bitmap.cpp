#include "bitmap.h"

#include <array>
#include <fstream>
#include <iostream>

#include "configuration.h"
/* due to get_mask */
#include "chunker.h"

namespace bmp {

image::image(const std::string &filename, uint8_t chunk_size)
    : filename(filename)
    , chunk_size(chunk_size)
    , cells_per_byte(8 / chunk_size) {}

bool image::assign_input() {
    auto ifstream = std::make_unique<std::ifstream>(filename, std::ios::binary);
    if (ifstream == nullptr || !ifstream->is_open() || !ifstream->good()) {
        return false;
    }
    this->input = std::move(ifstream);
    return true;
}

bool image::assign_input(std::unique_ptr<std::istream> input) {
    this->input = std::move(input);
    return this->input != nullptr;
}

static uint16_t to_uint16(const uint8_t *data) {
    return static_cast<uint16_t>(data[0]) |
           (static_cast<uint16_t>(data[1]) << 8);
}

static uint32_t to_uint32(const uint8_t *data) {
    return static_cast<uint32_t>(data[0]) |
           (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) |
           (static_cast<uint32_t>(data[3]) << 24);
}

static uint8_t count_padding(auto width, auto channels) {
    /* & 0b11 == % 4 */
    return (4 - (width * channels) & 0b11) & 0b11;
}

bool image::load_header(std::ostream &err) {
    const auto smaller_header_size = 14u;
    header.resize(smaller_header_size);

    if (!input->read(reinterpret_cast<char *>(header.data()),
                     smaller_header_size)) {
        err << "there was an error while reading file " << filename
            << ", bytes read: " << input->gcount() << '\n';
        return false;
    }
    if (header[0] != 'B' || header[1] != 'M') {
        err << "file " << filename << " has invalid magic number to be bmp file"
               " (expected BM, got " << header[0] << header[1] << ")\n";
        return false;
    }

    data_offset = to_uint32(header.data() + 10);
    if (data_offset <= smaller_header_size) {
        err << "data offset in header of " << filename
            << " is too small to be bmp\n";
        return false;
    }

    auto file_size = to_uint32(header.data() + 2);
    if (file_size < data_offset) {
        err << "the file size in header of " << filename
            << " is smaller than data offset\n";
        return false;
    }

    header.resize(data_offset);
    if (!input->read(reinterpret_cast<char *>(header.data()) + smaller_header_size,
                       data_offset - smaller_header_size)) {
        err << "there was an error while reading file " << filename
            << ", bytes read: " << input->gcount() << '\n';
        return false;
    }
    width = to_uint32(header.data() + 18);
    height = to_uint32(header.data() + 22);

    uint16_t bit_count = to_uint16(header.data() + 28);
    if (bit_count != 24 && bit_count != 32) {
        err << "file " << filename << " has unsupported bit count: "
            << bit_count << " (only 24 and 32 are supported)\n";
        return false;
    }
    channel_count = bit_count / 8;
    capacity = width * channel_count * height;
    /* 4 * because chunk_size for metadata will be always 2 */
    if (capacity <= 4 * HIDDEN_METADATA_SIZE) {
        err << "file " << filename << " is too small to hide any data\n";
        return false;
    }
    capacity -= 4 * HIDDEN_METADATA_SIZE;

    padding = count_padding(width, channel_count);

    uint32_t compression = to_uint32(header.data() + 30);
    if (compression) {
        err << "file " << filename << " is compressed\n";
        return false;
    }
    return true;
}

std::string image::get_output_path() {
    using namespace std::string_literals;

    auto basename_index = filename.rfind('/');
    if (basename_index == std::string::npos)
        basename_index = 0;
    else
        ++basename_index;
    auto basename = filename.substr(basename_index);

    return "bitmaps_out/"s + basename;
}

bool image::write_header_to_output() {
    if (!output || !output->good())
        return false;
    output->write(reinterpret_cast<char *>(header.data()), header.size());
    return output->good();
}

bool image::assign_output() {
    auto ofstream = std::make_unique<std::ofstream>(
        get_output_path(), std::ios::binary);
    if (ofstream == nullptr || !ofstream->is_open() || !ofstream->good()) {
        return false;
    }
    this->output = std::move(ofstream);
    return true;
}

bool image::assign_output(std::unique_ptr<std::ostream> output) {
    this->output = std::move(output);
    return this->output != nullptr;
}

auto image::operator<=>(const image &rhs) const {
    return this->seq <=> rhs.seq;
}

void image::set_data_start() {
    this->input->seekg(this->data_offset, std::ios::beg);
}

std::size_t image::byte_capacity() const {
    return capacity / cells_per_byte;
}

image_buffer::image_buffer(bmp::image &im, uint8_t chunk_size)
    : im(im)
    , mask(get_mask(chunk_size))
    , erase_mask(~mask) {
    im.set_data_start();
}

bool image_buffer::hide_chunk(uint8_t chunk) {
    if (!move_index([this]() { return write_and_read(); }))
        return false;

    buffer[index] &= erase_mask;
    buffer[index++] |= chunk;
    ++x;
    return true;
}

bool image_buffer::extract_chunk(uint8_t &chunk) {
    if (!move_index([this]() { return read(); }))
        return false;

    chunk = buffer[index++] & mask;
    ++x;
    return true;
}

void image_buffer::change_chunk_size(uint8_t chunk_size) {
    mask = get_mask(chunk_size);
    erase_mask = ~mask;
}

void image_buffer::copy_rest() {
    while (write_and_read()) {}
}

bool image_buffer::read() {
    buffer.fill(static_cast<char>(0));
    im.input->read(buffer.data(), BUFFER_SIZE);
    loaded = im.input->gcount();
    index = 0;
    return loaded > 0;
}

bool image_buffer::write_and_read() {
    if (loaded > 0)
        im.output->write(buffer.data(), loaded);
    return read();
}

bool image_buffer::move_index(std::function<bool(void)> read_or_writeread) {
    while (true) {
        if (index >= loaded) {
            if (!read_or_writeread())
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
    return true;
}

}
