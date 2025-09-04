#ifndef EXTRACT_H
#define EXTRACT_H

#include <string>

#include "../include/bitmap.h"

struct extracted {
    bool success;
    std::string error_message;

    operator bool() const
    {
        return success;
    }
};

extracted  extract_data(bmp::image &im, std::vector<unsigned char> &data,
    unsigned char id);

#endif  // EXTRACT_H
