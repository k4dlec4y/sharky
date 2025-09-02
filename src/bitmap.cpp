#include <fstream>

#include "../include/bitmap.h"

const int FST_HEAD_SIZE = 14;
const int HIDE_METADATA_SIZE = 52;

namespace bmp {

struct bad_bmp_format {
    std::string filename;
    std::string message;
};

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

static int64_t chars_to_int64(const unsigned char *data)
{
    return static_cast<int64_t>(data[0]) |
           (static_cast<int64_t>(data[1]) << 8) |
           (static_cast<int64_t>(data[2]) << 16) |
           (static_cast<int64_t>(data[3]) << 24) |
           (static_cast<int64_t>(data[4]) << 32) |
           (static_cast<int64_t>(data[5]) << 40) |
           (static_cast<int64_t>(data[6]) << 48) |
           (static_cast<int64_t>(data[7]) << 56);
}

static int count_padding(int width)
{
    return (4 - width % 4) % 4;
}

image read_bmp(const char *filename)
{
    using namespace std::string_literals;

    image im(filename);
    im.header.resize(FST_HEAD_SIZE);

    std::ifstream input(im.filename, std::ios::binary);
    if (!input)
        throw bad_bmp_format{im.filename, "could not open the file\n"};

    input.seekg(0, std::ios::end);
    std::streamsize file_size = input.tellg();
    input.seekg(0, std::ios::beg);

    if (file_size < FST_HEAD_SIZE)
        throw bad_bmp_format{im.filename, "file is too small to be bmp\n"};
    if (!input.read(reinterpret_cast<char *>(im.header.data()), FST_HEAD_SIZE))
        throw bad_bmp_format{im.filename, "could not read\n"};

    if (im.header[0] != u'B' || im.header[1] != u'M')
        throw bad_bmp_format{im.filename, "invalid signature for bmp file:"s +
            static_cast<char>(im.header[0]) + static_cast<char>(im.header[1]) +
            '\n'};

    if (chars_to_int32(im.header.data() + 2) != file_size)
        throw bad_bmp_format{im.filename, "the actual size and size in bmp "
                                          "header does not match\n"};

    int32_t data_offset = chars_to_int32(im.header.data() + 10);
    im.header.resize(data_offset);
    if (!input.read(reinterpret_cast<char *>(im.header.data()) + FST_HEAD_SIZE,
                    data_offset - FST_HEAD_SIZE))
        throw bad_bmp_format{im.filename, "could not read\n"};

    im.width = chars_to_int32(im.header.data() + 18);
    im.height = chars_to_int32(im.header.data() + 22);
    im.channel_count = chars_to_int16(im.header.data() + 28) / 8;
    if (im.channel_count != 24 && im.channel_count != 32)
        throw bad_bmp_format{im.filename, "invalid bit count for single pixel,"
                                          "use 24/32\n"};
    im.padding = count_padding(im.width * im.channel_count);

    im.capacity = static_cast<int64_t>(im.width) * im.height * (im.channel_count / 4);
    if (im.capacity <= HIDE_METADATA_SIZE)
        throw bad_bmp_format{im.filename, "image size is too small\n"};

    int32_t compression = chars_to_int32(im.header.data() + 30);
    if (compression)
        throw bad_bmp_format{im.filename, "image is compressed\n"};

    std::size_t img_size = file_size - data_offset;
    im.img_data.resize(img_size);
    if (input.read(reinterpret_cast<char *>(im.img_data.data()), img_size).fail())
        throw bad_bmp_format{im.filename, "could not read\n"};

    return im;
}

}
