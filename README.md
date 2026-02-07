# sharky

## Table of Contents
- [Introduction](#introduction)
- [Overview](#overview)
- [Steganography Method](#steganography-method)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Limitations](#limitations)
- [License](#license)
- [Author](#author)

## Introduction
This is my first standalone C++ project and my first software project
developed beyond the university. It was created as a practical way
to get more comfortable with C++ concepts and language features that I learned
during my studies, and to apply them in a real, self-directed project.

## Overview
`sharky` is a command-line tool that embeds data into BMP images and later
extracts it without visibly altering the image. The tool operates directly on
the BMP pixel data and preserves the original image structure and metadata.

The program supports:
- Hiding binary data inside BMP images
- Extracting previously hidden data
- Verifying input image validity
- Safe handling of malformed or unsupported BMP files

Only uncompressed BMP images are supported.

## Steganography Method
The data hiding mechanism is based on **least significant bit (LSB)**
manipulation of pixel color channels. This allows data to be embedded while
keeping visual changes largely imperceptible to the human eye, depending on the
chosen encoding mode.

The tool supports multiple embedding modes, which control how many bits of each
pixel channel are used:
- **1-bit LSB**: Minimal visual impact; changes are effectively imperceptible
- **2-bit LSB**: Still visually indistinguishable in practice
- **4-bit LSB**: Noticeable image degradation, but original content remains
  recognizable
- **8-bit replacement**: Pixel data is fully replaced, resulting in complete
  loss of the original image content

### Embedded Metadata Structure
Before the actual payload data, `sharky` embeds a fixed-size metadata header at
the beginning of the pixel data. This metadata is always encoded using **2 least
significant bits**, regardless of the selected payload embedding mode.

The metadata header is **9 bytes** long and has the following structure
(in order):

| Offset | Size | Description |
|------:|-----:|-------------|
| 0 | 1 byte | `'S'` magic byte identifying a Sharky-modified image |
| 1 | 1 byte | `'H'` magic byte identifying a Sharky-modified image |
| 2 | 1 byte | Hiding ID used to verify that images belong to the same embedding session |
| 3 | 1 byte | Sequence number indicating the extraction order when data is split across multiple images |
| 4 | 4 bytes | Size of the embedded payload in bytes (metadata size excluded) |
| 8 | 1 byte | Chunk size used for embedding (`1`, `2`, `4`, or `8` bits per channel) |

The magic bytes (`'S'`, `'H'`) allow the extractor to quickly identify whether an
image contains data embedded by `sharky`. The hiding ID and sequence number make
it possible to safely extract data spread across multiple images while ensuring
correct ordering. The payload size enables deterministic extraction without
sentinel values, and the chunk size field specifies how many bits per pixel
channel were used during embedding.

## Requirements
- C++20 compatible compiler (GCC or Clang, type it into the makefile)
- make
- Tested on Linux (Debian 13)

## Installation
```bash
git clone https://github.com/k4dlec4y/sharky
cd sharky
make
```

## Usage
`sharky` is a command-line tool that operates in one of two modes: **hiding**
or **extraction**. Exactly one mode must be selected.

### Modes
- `-h`, `--hide`  
  Enables hiding mode. The specified file is embedded into the provided BMP
  images.

- `-e`, `--extract`  
  Enables extraction mode. Hidden data is extracted from the provided BMP
  images and written to a file.

Hiding and extraction modes **cannot be used at the same time**.

### File selection
- `-f <path>`, `--file <path>`  
  Specifies the input file to hide (in hiding mode) or the output file where
  extracted data will be written (in extraction mode).

### Chunk size selection
- `-c <1|2|4|8>`, `--chunk_size <1|2|4|8>`  
  Selects how many bits per pixel channel are used for data embedding.
  The default chunk size is **2**.

The selected chunk size applies to **all images that follow it**, until another
`-c` option is encountered. This allows different images to use different
embedding modes in a single command.

For example:
- One image with chunk size 2 (default)
- Two images with chunk size 8
- One image with chunk size 4

```bash
image1.bmp -c 8 image2.bmp image3.bmp -c 4 image4.bmp
```

### Image arguments
All remaining arguments that are not options are treated as paths to BMP image
files. Only valid, uncompressed BMP images are accepted.

### Output
When hiding data, the modified images are written to:
```bash
sharky/output_bitmaps/
```
Each output image uses the original filename with the suffix .out appended.

### Error handling
In case of invalid arguments, unsupported files, or runtime errors, `sharky`
prints a descriptive error message to **standard error (`stderr`)** explaining
what went wrong. The program then terminates without performing partial hiding
or extraction.

### Examples
Hiding data
```bash
sharky -c 4 bitmaps/image.bmp -c 8 bitmaps/image2.bmp -h -f data/data_in
```
This command:
- Hides data/data_in into two images
- Uses 4-bit LSB encoding for image.bmp
- Uses 8-bit replacement for image2.bmp
- Writes modified images to sharky/output_bitmaps/

Extracting data
```bash
sharky -c 4 output_bitmaps/image2.bmp.out output_bitmaps/image.bmp.out -e -f data/data_out
```
This command:
- Extracts hidden data from the provided images
- Restores the original payload into data/data_out
- Uses embedded metadata to determine decoding parameters and image order (-c 4 is ignored)

## Limitations
- Only uncompressed BMP files are supported
- Image capacity limits the maximum size of embedded data
- No encryption is performed (steganography only, not cryptography)

## License
This project is licensed under the MIT License – see the LICENSE file for details.

## Author
Created by Matúš Kadlecay – https://github.com/k4dlec4y
