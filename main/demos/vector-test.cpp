#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <vector>
using namespace std;

vector<int> vic(100);


extern "C"
{
   void app_main();
}


void app_main() {
    
    uint16_t custom_size = 10000;
    // 1068 bytes
    /* 
    uint8_t static_buffer[custom_size];
     
    for (uint16_t i = 0; i<sizeof(static_buffer); ++i) {
        static_buffer[i] = 0xff;
    } */

    

    // Takes 1004 2072 of heap
    /* uint8_t* p = (uint8_t*) malloc(custom_size);
    memset(p, 1, custom_size); */

    // int     : 5072 of heap
    // uint8_t : 2164 of heap
    // Dynamic size vector
    vector<uint8_t> vic;
    vector<uint8_t>::iterator buffer_pos;
    buffer_pos = vic.begin();

    for (uint16_t i = 0; i<10; ++i) {
        // Hangs everything
        //vic.insert(buffer_pos+i, 0xff);
        vic.push_back(0xff);
    }

     //printf("Hello this is a Vector test\n");

     printf("Free heap: %d vector size: %d\n", xPortGetFreeHeapSize(), vic.size());
}