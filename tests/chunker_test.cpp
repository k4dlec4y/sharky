#include "chunker.h"
#include <gtest/gtest.h>

TEST(get_mask, returns_correct_mask) {
    EXPECT_EQ(get_mask(1), 0b01);
    EXPECT_EQ(get_mask(2), 0b11);
    EXPECT_EQ(get_mask(4), 0b1111);
    EXPECT_EQ(get_mask(8), 0b11111111);
}

TEST(chunker, splits_correctly_2bit_chunks) {
    std::vector<uint8_t> data{0b10101010, 0b11110000};
    chunker chkr{data, 2};

    uint8_t chunk;
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b10);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b10);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b10);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b10);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b00);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b00);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b11);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b11);
    EXPECT_FALSE(chkr.get_chunk(chunk));
}

TEST(chunker, splits_correctly_8bit_chunks) {
    std::vector<uint8_t> data{0b10101010, 0b11110000};
    chunker chkr{data, 8};

    uint8_t chunk;
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b10101010);
    EXPECT_TRUE(chkr.get_chunk(chunk));
    EXPECT_EQ(chunk, 0b11110000);
    EXPECT_FALSE(chkr.get_chunk(chunk));
}

TEST(chunker, merges_correctly_2bit_chunks) {
    std::vector<uint8_t> data{0, 0};
    chunker chkr{data, 2, false};

    EXPECT_TRUE(chkr.send_chunk(0b10));
    EXPECT_TRUE(chkr.send_chunk(0b10));
    EXPECT_TRUE(chkr.send_chunk(0b10));
    EXPECT_TRUE(chkr.send_chunk(0b10));
    EXPECT_TRUE(chkr.send_chunk(0b00));
    EXPECT_TRUE(chkr.send_chunk(0b00));
    EXPECT_TRUE(chkr.send_chunk(0b11));
    EXPECT_TRUE(chkr.send_chunk(0b11));
    EXPECT_FALSE(chkr.send_chunk(0b00));

    EXPECT_EQ(data[0], 0b10101010);
    EXPECT_EQ(data[1], 0b11110000);
}

TEST(chunker, merges_correctly_8bit_chunks) {
    std::vector<uint8_t> data{0, 0};
    chunker chkr{data, 8, false};

    EXPECT_TRUE(chkr.send_chunk(0b10101010));
    EXPECT_TRUE(chkr.send_chunk(0b11110000));
    EXPECT_FALSE(chkr.send_chunk(0b00000000));

    EXPECT_EQ(data[0], 0b10101010);
    EXPECT_EQ(data[1], 0b11110000);
}
