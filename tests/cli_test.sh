#!/bin/bash
set -e

rm -f data/data_out
mkdir -p bitmaps_in bitmaps_out data

build/sharky \
    --hide \
    --chunk_size 4 \
    bitmaps_in/image.bmp \
    --chunk_size 8 \
    bitmaps_in/image2.bmp \
    --file data/data_in
build/sharky \
    --extract \
    bitmaps_out/image2.bmp \
    bitmaps_out/image.bmp \
    --file data/data_out

echo "Comparing input/output data..."
cmp data/data_in data/data_out
echo "Test passed"
