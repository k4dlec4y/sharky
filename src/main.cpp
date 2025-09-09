#include <iostream>
#include <string>

#include "../include/bitmap.h"
#include "../include/hide.h"
#include "../include/extract.h"
#include "../include/info_structs.h"

const int NO_MODE = 0;
const int HIDE = 1;
const int EXTRACT = 2;
const std::size_t CHUNK_COUNT = 4;

int process_args(std::vector<std::string> &args, std::vector<bmp::image> &images,
    std::string &message_filename)
{
    int mode = NO_MODE;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--hide" || args[i] == "-h") {
            if (mode == EXTRACT) {
                std::cerr << "cannot use hide and extract at the same time" << '\n';
                return 0;
            }
            mode = HIDE;
        } else if (args[i] == "--extract" || args[i] == "-e") {
            if (mode == HIDE) {
                std::cerr << "cannot use hide and extract at the same time" << '\n';
                return 0;
            }
            mode = EXTRACT;
        } else if (args[i] == "--file" || args[i] == "-f") {
            if (++i == args.size()) {
                std::cerr << "-f or --file was used as the last argument" << '\n';
                return 0;
            }
            message_filename = args[i];
        } else {
            images.emplace_back(bmp::image(args[i]));

            if (!images.back().in.is_open()) {
                std::cerr << "image " << args[i] << " could not be opened" << '\n';
                images.pop_back();
                continue;
            } else if (!bmp::read_header(images.back(), CHUNK_COUNT)) {
                images.pop_back();
                continue;
            }

            std::cout << "image " << args[i] << " was successfully opened "
                "with byte_capacity: " << images.back().byte_capacity << '\n';
        }
    }
    if (mode == 0)
        std::cerr << "no mode was selected\n";
    if (message_filename == "") {
        std::cerr << "could not find message file -> tag one with -f/--file\n";
        return NO_MODE;
    }
        
    return mode;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string message("");
    std::vector<bmp::image> images;

    switch (process_args(args, images, message))
    {
    case HIDE:
        return 0;
    case EXTRACT:
        return 0;
    default:
        return 1;
    }
}
