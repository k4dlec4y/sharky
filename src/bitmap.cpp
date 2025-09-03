#include <fstream>
#include <filesystem>
#include <iostream>

#include "../include/bitmap.h"

const int FST_HEAD_SIZE = 14;
const int HIDDEN_METADATA_SIZE = 7;

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
    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    return file_size;
}

image read_bmp(const char *filename)
{
    using namespace std::string_literals;

    image im(filename);
    im.header.resize(FST_HEAD_SIZE);

    std::ifstream input(im.filename, std::ios::binary);
    if (!input)
        throw bad_format{im.filename, "could not open the file\n"};

    std::streamsize file_size = get_file_size(input);

    if (file_size < FST_HEAD_SIZE)
        throw bad_format{im.filename, "file is too small to be bmp\n"};
    if (!input.read(reinterpret_cast<char *>(im.header.data()), FST_HEAD_SIZE))
        throw bad_format{im.filename, "could not read\n"};

    if (im.header[0] != u'B' || im.header[1] != u'M')
        throw bad_format{im.filename, "invalid signature for bmp file:"s +
            static_cast<char>(im.header[0]) + static_cast<char>(im.header[1]) +
            '\n'};

    if (chars_to_int32(im.header.data() + 2) != file_size)
        throw bad_format{im.filename, "the actual size and size in bmp "
                                          "header does not match\n"};

    int32_t data_offset = chars_to_int32(im.header.data() + 10);
    im.header.resize(data_offset);
    if (!input.read(reinterpret_cast<char *>(im.header.data()) + FST_HEAD_SIZE,
                    data_offset - FST_HEAD_SIZE))
        throw bad_format{im.filename, "could not read\n"};

    im.width = chars_to_int32(im.header.data() + 18);
    im.height = chars_to_int32(im.header.data() + 22);

    int16_t bit_count = chars_to_int16(im.header.data() + 28);
    if (bit_count != 24 && bit_count != 32)
        throw bad_format{im.filename, "invalid bit count for single pixel,"
                                      "use 24/32\n"};
    im.channel_count = bit_count / 8;
    im.padding = count_padding(im.width * im.channel_count);

    im.byte_capacity = static_cast<std::size_t>(im.width) * im.height *
                       im.channel_count / 4;
    if (im.byte_capacity <= HIDDEN_METADATA_SIZE)
        throw bad_format{im.filename, "image size is too small\n"};

    int32_t compression = chars_to_int32(im.header.data() + 30);
    if (compression)
        throw bad_format{im.filename, "image is compressed\n"};

    std::size_t img_size = file_size - data_offset;
    im.img_data.resize(img_size);
    if (input.read(reinterpret_cast<char *>(im.img_data.data()), img_size).fail())
        throw bad_format{im.filename, "could not read\n"};

    return im;
}

bool write_bmp(image &im)
{
    using namespace std::string_literals;

    std::size_t basename_index = im.filename.rfind('/');
    if (basename_index == std::string::npos)
        basename_index = 0;
    else
        ++basename_index;
    std::string basename = im.filename.substr(basename_index);

    std::filesystem::create_directories("output_bitmaps"s);

    std::ofstream out("output_bitmaps/"s + basename + ".out"s,
        std::ios::binary | std::ios::trunc);

    out.write(reinterpret_cast<char *>(im.header.data()), im.header.size());
    out.write(reinterpret_cast<char *>(im.img_data.data()), im.img_data.size());
    return out.good();
}

}
