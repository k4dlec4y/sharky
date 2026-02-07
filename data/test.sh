#!/bin/bash

./sharky -c 4 bitmaps/image.bmp -c 8 bitmaps/image2.bmp -h -f data/data_in &&
    ./sharky -c 4 output_bitmaps/image2.bmp.out output_bitmaps/image.bmp.out -e -f data/data_out
cmp data/data_in data/data_out
ex=$?
exit $ex
