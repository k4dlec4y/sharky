#include <fstream>
#include <filesystem>
#include <iostream>

#include "../include/bitmap.h"

const int smaller_header_size = 14;

namespace bmp {

static int16_t to_int16(const uint8_t *data)
{
    return static_cast<int16_t>(data[0]) |
           (static_cast<int16_t>(data[1]) << 8);
}

static int32_t to_int32(const uint8_t *data)
{
    return static_cast<int32_t>(data[0]) |
           (static_cast<int32_t>(data[1]) << 8) |
           (static_cast<int32_t>(data[2]) << 16) |
           (static_cast<int32_t>(data[3]) << 24);
}

static int count_padding(int width)
{
    return (4 - width % 4) % 4;
}

std::streamsize get_file_size(std::ifstream &file)
{
    auto orig = file.tellg();
    file.seekg(0, std::ios::end);
    auto file_size = file.tellg();
    file.seekg(orig, std::ios::beg);
    return file_size;
}

bool load_header(image &im, std::size_t cells_per_byte)
{
    using namespace std::string_literals;

    im.header.resize(smaller_header_size);

    auto file_size = get_file_size(im.input);

    if (file_size < smaller_header_size) {
        std::cerr << "file " << im.filename << " is too small to be bmp\n";
        return false;
    }
    if (!im.input.read(reinterpret_cast<char *>(im.header.data()), smaller_header_size)) {
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
    im.padding = count_padding(im.width * im.channel_count);

    im.byte_capacity = (static_cast<std::size_t>(im.width) * im.channel_count
                        - im.padding) * im.height / cells_per_byte;
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

bool image::open_ofstream()
{
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

}
