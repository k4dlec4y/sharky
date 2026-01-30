#include <iostream>
#include <string>
#include <string_view>

#include "../include/configuration.h"
#include "../include/bitmap.h"
#include "../include/hide.h"
#include "../include/extract.h"

enum mode { NO_MODE, HIDE, EXTRACT };
constexpr auto cells_per_byte = 8 / chunk_size;

mode process_args(
    std::vector<std::string> &args,
    std::vector<bmp::image> &images,
    std::string &data_filename
) {
    using namespace std::literals;
    mode m = mode::NO_MODE;

    for (auto i = 0u; i < args.size(); ++i) {

        if (args[i] == "--hide"sv || args[i] == "-h"sv) {
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
            images.emplace_back(bmp::image(args[i]));

            if (!images.back().input.is_open()) {
                std::cerr << "image " << args[i] << " could not be opened\n";
                images.pop_back();
            } else if (!bmp::load_header(images.back(), cells_per_byte)) {
                /* error line is printed in load_header() */
                images.pop_back();
            } else {
                std::cout << "image " << args[i]
                          << " was opened with byte_capacity: "
                          << images.back().byte_capacity - hidden_metadata_size << '\n';
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

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);
    std::vector<bmp::image> images;

    std::string data_filename("");
    
    switch (process_args(args, images, data_filename))
    {
    case mode::HIDE:
        return hide(images, data_filename);
    case mode::EXTRACT:
        return extract(images, data_filename);
    default:
        return 1;
    }
}
