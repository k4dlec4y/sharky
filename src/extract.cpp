#include <cassert>
#include <vector>
#include <span>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <iostream>

#include "configuration.h"
#include "chunker.h"
#include "bitmap.h"
#include "extract.h"

static void run_out_of_bytes_error_log(
    std::ostream &os,
    std::string_view filename
) {
    os << "image file " << filename << " run out of bytes too early!\n";
}

static void invalid_magic_number_log(
    std::ostream &os,
    std::string_view filename,
    uint8_t byte1,
    uint8_t byte2
) {
    os << "image " << filename << " has invalid sharky magic number! ("
       << std::hex << byte1 << ", " << byte2 << std::dec << ")\n";
}

static void invalid_seq_number_log(
    std::ostream &os,
    std::string_view filename,
    uint8_t seq,
    uint8_t expected_seq
) {
    os << "image " << filename << " has invalid seq number! ("
       << seq << ", expected: " << expected_seq << ")\n";
}

static void invalid_id_log(
    std::ostream &os,
    std::string_view filename,
    uint8_t id1,
    uint8_t id2
) {
    os << "image " << filename << " has different id than other images! ("
       << id1 << ", expected: " << id2 << ")\n";
}

static bool extract_bytes(
    bmp::image_buffer& buffer,
    chunker& chunker,
    uint8_t chunk_size,
    auto size,
    std::string_view filename,
    std::ostream& err
) {
    uint8_t chunk;
    for (auto _ = 0u; _ < size * (8 / chunk_size); ++_) {
        if (!buffer.extract_chunk(chunk) || !chunker.send_chunk(chunk)) {
            run_out_of_bytes_error_log(err, filename);
            return false;
        }
    }
    return true;
}

bool extract_hidden_metadata(
    bmp::image& im,
    bmp::image_buffer& buffer,
    std::ostream& err
) {
    std::vector<uint8_t> data(HIDDEN_METADATA_SIZE);
    chunker chunker{std::span(data.data(), data.size()), MD_CHUNK_SIZE, false};

    if (!extract_bytes(buffer, chunker, MD_CHUNK_SIZE,
                       HIDDEN_METADATA_SIZE, im.filename, err))
        return false;

    if (data[0] != 'S' || data[1] != 'H') {
        invalid_magic_number_log(err, im.filename, data[0], data[1]);
        return false;
    }

    im.id = data[2];
    im.seq = data[3];

    im.hidden_data_size = 0u;
    for (std::size_t i = 0; i < 4; ++i)
        im.hidden_data_size |= data[4 + i] << (i * 8);

    im.chunk_size = data[8];
    return true;
}

bool extract_data(
    bmp::image& im,
    bmp::image_buffer& buffer,
    std::span<uint8_t> data,
    std::ostream& err
) {
    chunker chunker{data, im.chunk_size, false};
    if (!extract_bytes(buffer, chunker, im.chunk_size,
        data.size(), im.filename, err))
        return false;

    return true;
}

int extract(
    std::vector<bmp::image>& images,
    std::ostream& data_ostream,
    std::ostream& err
) {
    assert(images.size() > 0);
    std::vector<bmp::image_buffer> buffers{};
    auto data_size = 0;

    for (auto i = 0u; i < images.size(); ++i) {
        buffers.emplace_back(images[i], MD_CHUNK_SIZE);
        if (!extract_hidden_metadata(images[i], buffers[i], err))
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
            invalid_seq_number_log(err, im.filename, im.seq, i);
            return 1;
        }
    }

    uint8_t id = images[indx[0]].id;
    for (auto &im : images) {
        if (im.id != id) {
            invalid_id_log(err, im.filename, im.id, id);
            return 1;
        }
    }

    std::vector<uint8_t> data(data_size);
    auto data_index = 0;
    for (auto i = 0u; i < images.size(); ++i) {
        auto j = indx[i];
        auto n = images[j].hidden_data_size;

        buffers[j].change_chunk_size(images[j].chunk_size);
        if (!extract_data(images[j], buffers[j],
                          std::span(data.data() + data_index, n), err))
            return 1;
        data_index += n;
    }

    return data_ostream.write(reinterpret_cast<char *>(data.data()),
                              data.size()).fail();
}
