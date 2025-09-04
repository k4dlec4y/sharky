#ifndef HIDE_H
#define HIDE_H

bool hide_data(bmp::image &im, std::string_view data,
    std::size_t &data_index, unsigned char id, unsigned char seq);

#endif  // HIDE_H
