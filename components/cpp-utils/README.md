# About this Repository

This is not my work. All credits go to NKolban great [esp32-snippets Repository](https://github.com/nkolban/esp32-snippets)
All that I did is to copy his files and enable them to be added as a ESP-IDF submodule. To do so, just use git submodules:

    git submodule add https://github.com/martinberlin/cpp-utils.git components/cpp-utils

And this great resource should be compiled and available in your ESP-IDF Project.
I may update this from time to time if I find compiling errors, just file an Issue, and add how to reproduce it, if you find one.

### Compiling with warnings

    [10/10] Generating binary image from built executable
    esptool.py v3.0-dev
    Generated /home/martin/esp/examples/cale-idf/build/hello-world.bin
    Project build complete.

Please note that Socket component needs to be refactored so all components depending on it have been renamed to *.cpp.txt and won't be available to be included in your project:

    Socket.h
    FTPServer.h
    HttpParser.h
    HttpResponse.h
    HttpRequest.h
    PubSubClient.h
    TFTP.h
    WebSocket.h
    WebSocketFileTransfer.h

If someone forks this and fixes the Socket component then we could make this ones work again.
Pull requests are welcome!

### Enabling C++ exception handling

Note from **martinberlin**
If you get this when compiling:

error: #error "C++ exception handling must be enabled within make menuconfig. See Compiler Options > Enable C++ Exceptions."

Follow the recommendation and enable the C++ exceptions using menuconfig

    idf.py -D IDF_TARGET=esp32 menuconfig

After this line nkolban original Readme file is copied without any alterations:

# CPP Utils
This directory contains a wealth of C++ classes that have been found useful when working in C++ in conjunction
with the ESP-IDF.  The classes have been documented using `doxygen` so one can run a doxygen processor over them
to create the user guides and programming references.

# Compiling the C++ classes
The C++ classes found here exist as an ESP-IDF component.  To build the classes and then use them in your project perform the following
steps:

1. Create an ESP-IDF project.
2. Create a directory called `components` in the root of your ESP-IDF project.
3. Copy this directory (`cpp_utils`) into your new `components` directory.  The result will be `<project>/components/cpp_utils/<files>`.
4. In your ESP-IDF project build as normal.

The C++ classes will be compiled and available to be used in your own code.

# Adding a main function
When working with C++, your calling function should also be written in C++.  Consider replacing your `main.c` with the following
`main.cpp` file:

```
extern "C" {
   void app_main();
}

void app_main() {
   // Your code goes here
}
```

The way to read the above is that we are defining a global function called `app_main` but we are saying that its external
linkage (i.e. how it is found and called) is using the C language convention.  However, since the source file is `main.cpp` and
hence compiled by the C++ compiler, you can utilize C++ classes and language features within and, since it has C linkage, it will
satisfy the ESP-IDF environment as the entry point into your own code.

## BLE Functions
The Bluetooth BLE functions are only compiled if Bluetooth is enabled in `make menuconfig`.  This is primarily because
the ESP-IDF build system has chosen to only compile the underlying BLE functions if Bluetooth is enabled.

## Building the Documentation
The code is commented using the Doxygen tags.  As such we can run Doxygen to generate the data.  I use `doxywizard` using
the `Doxyfile` located in this directory.

## Building the Arduino libraries
Some of the classes in this package also have applicability in an Arduino environment.  A `Makefile` called `Makefile.arduino` is provided to build the libraries.  For example:

```
$ make -f Makefile.arduino
```

The results of this will be ZIP files found in the `Arduino` directory relative to this one.  Targets include:

* `build_ble` - Build the BLE libraries. See also: [Arduino BLE Support](ArduinoBLE.md) .
