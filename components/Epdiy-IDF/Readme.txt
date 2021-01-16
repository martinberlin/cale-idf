The idea is to refactor original class of Epdiy:

https://github.com/vroland/epdiy

As a C++ class to inject into this parallel data-bus displays. First target to drive is the 4.7 inch epaper from LILYGO

ED047TC1 	4.7" 	960 x 540 pixels

As a blueprint I can imagine this is going to happen in the same way as SPI right now. As an example:

- - - - - - - - - - - - - - - - 
// Include header
#include <ED047TC1.h>

// Instantiate the parallel I2S Input/Output class that in turn has it's own Kconfig for the 8 GPIOs + clock
EpdI2SBus io;
ED047TC1 display(io);

// Do stuff with the epaper
display.println("Hello world");
display.update();

- - - - - - - - - - - - - - - - 

This directory is empty since the class is still in the conception phase and holds only the beginning of the Kconfig PIN configurations.
