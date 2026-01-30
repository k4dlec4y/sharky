#ifndef HIDE_H
#define HIDE_H

#include <span>

/**
 * @brief Hides data part into a single image.
 * 
 * @param im image into which data will be hidden
 * @param to_hide data part to be hidden
 * @param id id of hidding
 * @param seq data part index, used to later extract data in order
 * 
 * @return `true` on success, `false` on failure
 */
bool hide_data(
    bmp::image &im,
    std::span<uint8_t> to_hide,
    uint8_t id,
    uint8_t seq
);

/**
 * @brief Hides message (data) from file into images.
 * 
 * @param images vector of images, where text will be hidden
 * @param message_path file path of the message file
 * 
 * @return 0 on success, 1 if the full message could not be hidden
 * due to small images sizes, 2 in case of stream errors
 */
int hide(std::vector<bmp::image> &images, std::string message_path);

#endif  // HIDE_H
