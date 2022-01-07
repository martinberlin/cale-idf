## Video demo

This is demo that should be compiled independantly from the root folder since uses files stored in SPIFFs.

So just run idf.py build in this folder.

## Encoding videos in grayscale

Here there is an [interesting documentation](https://newbedev.com/how-to-encode-grayscale-video-streams-with-ffmpeg) about doing this with ffmpeg.

To be tried:

   PIX_FMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
   PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
   PIX_FMT_GRAY8A,    ///< 8bit gray, 8bit alpha
   