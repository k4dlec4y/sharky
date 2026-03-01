#!/bin/bash

./sharky -c 4 bitmaps_in/image.bmp -c 8 bitmaps_in/image2.bmp -h -f data/data_in &&
./sharky bitmaps_out/image2.bmp bitmaps_out/image.bmp -e -f data/data_out
cmp data/data_in data/data_out
ex=$?
exit $ex
