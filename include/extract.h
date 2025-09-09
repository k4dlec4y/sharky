#ifndef EXTRACT_H
#define EXTRACT_H

#include <string>

#include "../include/bitmap.h"
#include "../include/info_structs.h"

struct extracted {
    /* = 0 in case of error */
    std::size_t data_size{0};
    std::string error_message{""};

    operator bool() const
    {
        return data_size;
    }
};


/**
 * Extracts and checks important information about hidden data from the image
 * file and stores it into image struct.
 * 
 * @param im in-out parameter, contains information about image, after
 * the call `id` and `seq` will be set
 * @param p in-out parameter, information about current padding state,
 * is later passed to `extract_data()`
 * 
 * @return Struct with non-zero `data_size` member on success, otherwise
 * `data_size` is zero and `error_message` is set.
 * 
 * @note `data_size` represents byte amount of hidden message.
 */
extracted read_hidden_header(bmp::image &im, padding_info &p);

/**
 * Extracts hidden message from the image file.
 * 
 * @param im contains information about image
 * @param data reference to the vector of bytes, where hidden message will be
 * emplaced
 * @param data_index index from which data will be emplaced
 * @param data_end_index the lowest invalid index higher than `data_index`
 * for data emplacing
 * @param p information about current padding state after reading hidden header
 * 
 * @return `true` on success, `false` otherwise.
 */
bool extract_data(bmp::image &im, std::vector<unsigned char> &data,
    std::size_t data_index, std::size_t data_end_index, padding_info &p);

#endif  // EXTRACT_H
