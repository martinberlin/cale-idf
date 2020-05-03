# clean and build again for S2
rm -fr build/*
idf.py -D IDF_TARGET=esp32s2beta menuconfig
idf.py build
