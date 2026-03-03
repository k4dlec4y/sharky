#include <iostream>
#include <string>
#include <string_view>

#include "../include/configuration.h"
#include "../include/bitmap.h"
#include "../include/hide.h"
#include "../include/extract.h"

enum mode { NO_MODE, HIDE, EXTRACT };

mode process_args(
    std::vector<std::string> &args,
    std::vector<bmp::image> &images,
    std::string &data_filename
) {
    using namespace std::literals;
    mode m = mode::NO_MODE;
    uint8_t chunk_size = 2;

    for (auto i = 0u; i < args.size(); ++i) {
        if (args[i] == "--chunk_size"sv || args[i] == "-c"sv) {
            if (++i == args.size()) {
                std::cerr << "-c|--chunk_size was used as the last argument\n";
                return mode::NO_MODE;
            }
            try {
                chunk_size = static_cast<uint8_t>(std::stoi(args[i]));
                if (8 % chunk_size != 0) {
                    std::cerr << "supported chunk_size values are: 1, 2, 4, 8\n";
                    return mode::NO_MODE;
                }
            }
            catch(const std::exception& _) {
                std::cerr << "could not convert given chunk_size into an integer\n";
                return mode::NO_MODE;
            }
        } else if (args[i] == "--hide"sv || args[i] == "-h"sv) {
            if (m == mode::EXTRACT) {
                std::cerr << "cannot use hide and extract at the same time\n";
                return mode::NO_MODE;
            }
            m = mode::HIDE;
        } else if (args[i] == "--extract"sv || args[i] == "-e"sv) {
            if (m == mode::HIDE) {
                std::cerr << "cannot use hide and extract at the same time\n";
                return mode::NO_MODE;
            }
            m = mode::EXTRACT;
        } else if (args[i] == "--file"sv || args[i] == "-f"sv) {
            if (++i == args.size()) {
                std::cerr << "-f or --file was used as the last argument\n";
                return mode::NO_MODE;
            }
            data_filename = args[i];
        } else {
            auto im = bmp::image(args[i], chunk_size);
            if (!im.assign_input()) {
                std::cerr << "image " << args[i] << " could not be opened\n";
            } else if (bmp::load_header(im)) {
                images.push_back(std::move(im));
            }
        }
    }
    if (m == mode::NO_MODE) {
        std::cerr << "no mode was selected\n";   
        return mode::NO_MODE;
    } else if (data_filename == "") {
        std::cerr << "could not find message select it with -f/--file\n";
        return mode::NO_MODE;
    } else if (images.size() == 0) {
        std::cerr << "no proper images to hide data were supplied\n";
        return mode::NO_MODE;   
    }
    return m;
}

static int open_output_files(std::vector<bmp::image> &images) {
    for (auto &im : images) {
        if (!im.assign_output()) {
            std::cerr << "could not open output file for image "
                      << im.filename << "\n";
            return 1;
        }
        if (!im.write_header_to_output()) {
            std::cerr << "could not write bmp header to output file for image "
                      << im.filename << "\n";
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);
    std::vector<bmp::image> images;

    std::string data_filename("");

    switch (process_args(args, images, data_filename))
    {
    case mode::HIDE: {
        std::ifstream data_in{data_filename, std::ios::binary};
        if (!data_in.is_open() || !data_in.good())
            return 1;
        if (open_output_files(images))
            return 1;
        return hide(images, data_in);
    }

    case mode::EXTRACT: {
        std::ofstream data_out{data_filename, std::ios::binary};
        if (!data_out.is_open() || !data_out.good())
            return 1;
        return extract(images, data_out);
    }
    default:
        return 1;
    }
}
