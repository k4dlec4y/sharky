#ifndef EXTRACT_H
#define EXTRACT_H

#include <string>

#include "../include/bitmap.h"

/**
 * Extract, check and load metadata about hidden data from the image
 * file and stores it into image struct.
 * 
 * @param im in-out parameter, contains information about image, after
 * the call `id`, `seq` and `hidden_data_size` will be set
 * @param buffer image buffer, stores ifstream information,
 * is later passed to `extract_data()`
 * 
 * @return `true` on success, `false` otherwise
 */
bool extract_hidden_metadata(bmp::image& im, bmp::image_buffer& buffer);

/**
 * Extracts hidden data/message from single image.
 * 
 * @param im contains information about image
 * @param buffer image buffer, stores ifstream information
 * @param data span of std::vector, where extracted data will be placed
 * 
 * @return `true` on success, `false` otherwise
 * 
 * @note `buffer` should point on the first data byte of the image
 * (buffer should be passed to `extract_hidden_metadata()` first)
 */
bool extract_data(
    bmp::image& im,
    bmp::image_buffer& buffer,
    std::span<uint8_t> data
);

/**
 * Extracts hidden data/message from images.
 * 
 * @param images reference to vector of images to be extracted
 * @param data reference to vector of uchars, where data will
 * be stored
 * 
 * @return `0` on success, `1` otherwise
 */
int extract(
    std::vector<bmp::image>& images,
    std::vector<uint8_t>& data
);

#endif  // EXTRACT_H
