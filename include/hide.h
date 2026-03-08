#ifndef HIDE_H
#define HIDE_H

#include <span>

#include "bitmap.h"

/**
 * @brief Hides data part into a single image.
 * 
 * @param im image into which data will be hidden
 * @param to_hide data part to be hidden
 * @param id id of hidding
 * @param seq data part index, used to later extract data in order
 * @param err output stream for error logging
 * 
 * @return `true` on success, `false` on failure
 */
bool hide_data(
    bmp_image &im,
    std::span<uint8_t> to_hide,
    uint8_t id,
    uint8_t seq,
    std::ostream &err
);

/**
 * @brief Hides message (data) from file into images.
 * 
 * @param images vector of images, where text will be hidden
 * @param data input stream of the message file
 * @param out output stream for info logging
 * @param err output stream for error logging
 * 
 * @return 0 on success, 1 if the full message could not be hidden
 * due to small images sizes, 2 in case of stream errors
 */
int hide(
    std::vector<bmp_image> &images,
    std::istream &data,
    std::ostream &out = std::cout,
    std::ostream &err = std::cerr
);

#endif  // HIDE_H
