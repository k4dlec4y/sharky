#include <fstream>
#include <filesystem>
#include <iostream>

#include "../include/bitmap.h"

const int FST_HEAD_SIZE = 14;

namespace bmp {

static int16_t chars_to_int16(const unsigned char *data)
{
    return static_cast<int16_t>(data[0]) |
           (static_cast<int16_t>(data[1]) << 8);
}

static int32_t chars_to_int32(const unsigned char *data)
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

static std::streamsize get_file_size(std::ifstream &file)
{
    std::streampos orig = file.tellg();
    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(orig, std::ios::beg);
    return file_size;
}

bool read_header(image &im, std::size_t chunk_count)
{
    using namespace std::string_literals;

    im.header.resize(FST_HEAD_SIZE);

    std::streamsize file_size = get_file_size(im.in);

    if (file_size < FST_HEAD_SIZE) {
        std::cerr << "file " << im.filename << "is too small to be bmp\n";
        return false;
    }
    if (!im.in.read(reinterpret_cast<char *>(im.header.data()), FST_HEAD_SIZE)) {
        std::cerr << "file " << im.filename << "could not be read\n";
        return false;
    }
    if (im.header[0] != u'B' || im.header[1] != u'M') {
        std::cerr << "file " << im.filename << "invalid magic number for bmp "
            "file: " << im.header[0] << im.header[1] << '\n';
        return false;
    }
    if (chars_to_int32(im.header.data() + 2) != file_size) {
        std::cerr << "file " << im.filename << " - the actual size and size "
            "in bmp header does not match\n";
        return false;
    }
    im.data_offset = chars_to_int32(im.header.data() + 10);
    im.header.resize(im.data_offset);
    if (!im.in.read(reinterpret_cast<char *>(im.header.data()) + FST_HEAD_SIZE,
                    im.data_offset - FST_HEAD_SIZE)) {
        std::cerr << "file " << im.filename << "could not be read\n";
        return false;
    }
    im.width = chars_to_int32(im.header.data() + 18);
    im.height = chars_to_int32(im.header.data() + 22);

    int16_t bit_count = chars_to_int16(im.header.data() + 28);
    if (bit_count != 24 && bit_count != 32) {
        std::cerr << "file " << im.filename << "invalid bit count for single "
            "pixel, use 24/32\n";
        return false;
    }
    im.channel_count = bit_count / 8;
    im.padding = count_padding(im.width * im.channel_count);

    im.byte_capacity = (static_cast<std::size_t>(im.width) * im.channel_count
                        - im.padding) * im.height / chunk_count;
    if (im.byte_capacity <= HIDDEN_METADATA_SIZE * chunk_count) {
        std::cerr << "file " << im.filename << "size is too small\n";
        return false;
    }
    int32_t compression = chars_to_int32(im.header.data() + 30);
    if (compression) {
        std::cerr << "file " << im.filename << "is compressed\n";
        return false;
    }
    return true;
}

bool image::open_ofstream()
{
    using namespace std::string_literals;

    std::size_t basename_index = filename.rfind('/');
    if (basename_index == std::string::npos)
        basename_index = 0;
    else
        ++basename_index;
    std::string basename = filename.substr(basename_index);

    std::filesystem::create_directories("output_bitmaps"s);

    out.open("output_bitmaps/"s + basename + ".out"s,
        std::ios::binary | std::ios::trunc);
    return out.good();
}

}
