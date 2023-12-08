# Final Project - Image Compression


## Overview

## Lossless image compression

The testings of lossless image compression uses an implemented compressing algorithm based on Huffman Endcoding. more specific details are as following:

1. Read in the BMP file in byte, keep monitoring the frequency of each btye. Since we check the occurance of bytes, the frequency table woule be limited to at most 256 elements.

2. Using the obtained frequency table, construct a huffman tree. A piority queue is used to speed up the process. Then, for each different byte value, a unnique code is generated, the more frequently the value appears in the original file, the shorter the code will be. 

3. Generate a header for the huffman tree so that when decompressing, the computer can reconstruct the huffman tree.

4. Write the length of the header, and the header to the output file. Replace each byte with its corresponding huffman code, and write to the output file.

## Lossy Image Compression

*work in progress*

The lossy compression was based on JPEG compression. The steps used here for compressing an uncompressed .bmp image are:

1. Convert RGB to YCbCr (illuminance Y and 2 color channels Cb/Cr). The color channels can be compressed more because the human eye is more sensitive to illuminance than color.

2. Perform a DCT (discrete cosine transform) on each 8x8 block of pixels in the image. Each element in the DCT output will be quantized by a different amount, with higher frequencies being quantized more since higher frequencies are less necessary for the overall image. This results in a lot of the high frequency numbers being 0 which makes the data more compressible by subsequent run length coding. 

3. Zero run length coding. This stores a string of data as a value and the number of zeros that come before that value. This has good compression for long strings of zeros (which we have a lot of in the higher frequencies of the DCT output). 
Note that actual JPEGs compress the DC term at [0][0] differently since it's typically large. JPEGs store the *difference* between subsequent blocks' DC terms, since they are typically quite similar, but here I just included it in the RLC for simplicity.

A full JPEG implementation would then do a specific type of Huffman coding after the zero-length coding. We implemented Huffman coding in the lossless compression section in a slightly different way than JPEG needs it done, so our lossy compression stops at run length coding. Also not implemented here is the standard format with all sorts of headers/metadata that JPEG requires. 

TODO: show example, cite sources

### Optimizations 

TODO: Threads, SIMD?

### Results

TODO

To compile current lossy_compression test:

g++ -pthread -march=native lossy_compression.cpp -o lossy_test.o