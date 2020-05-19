# CALE ESP-IDF beta

This is the beginning, and a very raw try, to make CALE compile in the Espressif IOT Development Framework. At the moment to explore how difficult it can be to pass an existing ESP32 Arduino framework project to a ESP-IDF based one and to measure how far we can go compiling this with Espressif's own dev framework. 
It will take some weeks to have a working example. The reason is that we would like to explore alternative libraries like [ESP32 IDF Epaper example](https://github.com/loboris/ESP32_ePaper_example) and to make a version that is compatible with the recently released ESP32-S2

## Branches

**tft_test**  Is the original SPI master example from ESP-IDF 4 just refactored as a C++ class.
**master**   -> Nothing for now, will be merged only when v1.0 is testeable
**idf-base** -> Making the components base, most actual branch

The aim is to learn good how to code and link classes as git submodules in order to program the epaper display driver the same way. The goal is to have a tiny "human readable" code in cale.cpp main file and that the rest is encapsulated in classes.

### Submodules

ESP-IDF uses relative locations as its submodules URLs (.gitmodules). So they link to GitHub. To update the submodules once you **git clone** this repository:

    git submodule update --init --recursive
    
to download the submodules (components) for this project.
Reminder for myself, in case you update the module library just execute:

    # pull all changes for the submodules
    git submodule update --remote

For the rest of the README please refer to the version in [master branch](https://github.com/martinberlin/cale-idf).