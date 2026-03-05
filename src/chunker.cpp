#include "chunker.h"

#include <ranges>

uint8_t get_mask(uint8_t chunk_size) {
    return chunk_size < 8
           ? static_cast<uint8_t>((1u << chunk_size) - 1)
           : 0xffu;
}

chunker::chunker(std::span<uint8_t> data, uint8_t chunk_size, bool is_splitting)
    : data(data)
    , chunk_size(chunk_size)
    , chunks((8 + chunk_size - 1) / chunk_size)
    , chunks_index(is_splitting ? chunks.size() : 0)
    , mask(get_mask(chunk_size)) {}

bool chunker::get_chunk(uint8_t &chunk) {
    if (chunks_index >= chunks.size()) {
        if (data_index >= data.size())
            return false;
        split_byte(data[data_index++]);
        chunks_index = 0;
    }
    chunk = chunks[chunks_index++];
    return true;
}

bool chunker::send_chunk(uint8_t chunk) {
    chunks[chunks_index++] = chunk;
    if (chunks_index >= chunks.size()) {
        if (data_index >= data.size())
            return false;
        merge_chunks();
        chunks_index = 0;
    }
    return true;
}

void chunker::split_byte(uint8_t byte) {
    for (auto& chunk : chunks) {
        chunk = byte & mask;
        byte >>= chunk_size;
    }
}

void chunker::merge_chunks() {
    uint8_t merged = 0;
    for (auto& chunk : std::ranges::reverse_view(chunks)) {
        merged <<= chunk_size;
        merged |= chunk;
    }
    data[data_index++] = merged;
}
