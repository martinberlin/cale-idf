#include "plasticlogic014.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
PlasticLogic014::PlasticLogic014(EpdSpi2Cs& dio): 
  Adafruit_GFX(PLOGIC014_WIDTH, PLOGIC014_HEIGHT),
  PlasticLogic(PLOGIC014_WIDTH, PLOGIC014_HEIGHT, dio), IO(dio)
{
  
  printf("PlasticLogic014() %d*%d  buffer_vector_size:%d (at start)\n",
  PLOGIC014_WIDTH, PLOGIC014_HEIGHT, _buffer.size());
}

// Destructor
PlasticLogic014::~PlasticLogic014() {
  IO.release();
}

//Initialize the display
void PlasticLogic014::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) {
      printf("PlasticLogic014::init(%d) bufferSize: %d width: %d height: %d\n", 
    debug, PLOGIC014_BUFFER_SIZE, PLOGIC014_WIDTH, PLOGIC014_HEIGHT);
    }
    initIO(debug);
    
    uint8_t size = getEPDsize();
    _setSize(size);
    printf("EPD size ID: %d\n", size);

    if (size != 14) {
      ESP_LOGE(TAG, "ATTENTION the size responded by the display: %d does not mach this class", size);
    }
    clearScreen();
    _wakeUp();

    //printf("Epaper temperature after wakeUp: %d °C\n", IO.readTemp());
    //Set landscape mode as default
    setEpdRotation(1);
}

/**
 * This has the function of assigning the original Vector size
 */ 
void PlasticLogic014::clearScreen(){
  uint16_t x = 0;
  for (x = 0; x < PLOGIC014_BUFFER_SIZE; x++)
  {
    _buffer.push_back(0xff); //WHITE
  }

  printf("clearScreen() vector_size:%d RAM:%d\n", _buffer.size(), xPortGetFreeHeapSize());
}

void PlasticLogic014::update(uint8_t updateMode)
{
  ESP_LOGD(TAG, "Sending %d bytes buffer", sizeof(_buffer));

  uint8_t pixelAccessPos[3] = {EPD_PIXELACESSPOS, 0x00, 0x9f};
  uint8_t programMtp[2] = {EPD_PROGRAMMTP, 0x00};
  uint8_t displayEngine[2] = {EPD_DISPLAYENGINE, 0x03};

  IO.data(pixelAccessPos, sizeof(pixelAccessPos));

  _buffer.insert(_buffer.begin(), 0x10);
  IO.dataVector(_buffer);
  
  _waitBusy("Buffer sent", EPD_TMG_SRT);
  
  _powerOn();

    switch (updateMode) {
        case 0:
          IO.data(programMtp, sizeof(programMtp));
          IO.data(displayEngine, sizeof(displayEngine));
          _waitBusy("EPD_UPD_FULL", EPD_TMG_LNG);

          break;
        case EPD_UPD_PART:
          displayEngine[1] = 0x07;

          IO.data(programMtp, sizeof(programMtp));
          IO.data(displayEngine, sizeof(displayEngine));
          _waitBusy("EPD_UPD_PART", EPD_TMG_LNG);
            break;
        case EPD_UPD_MONO:
          programMtp[1] = 0x02;
          displayEngine[1] = 0x07;

          IO.data(programMtp, sizeof(programMtp));
          IO.data(displayEngine, sizeof(displayEngine));
          _waitBusy("EPD_UPD_MONO", EPD_TMG_LNG);
          break;
    }
    
  _powerOff();
}

void PlasticLogic014::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = PLOGIC014_WIDTH - x - 1;
      break;
    case 2:
      x = PLOGIC014_WIDTH - x - 1;
      y = PLOGIC014_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = PLOGIC014_HEIGHT - y - 1;
      break;
  }
  
  y=y+3;
  uint16_t pos = x/4 + (y) * _nextline;

  // check that is not going to write out of Vector bonds
  // #43 TODO: Check why is trying to update out of bonds anyways
  if (pos > _buffer.size()) {
    if (_vec_bonds_check) {
      printf("x:%d y:%d Vpos:%d >out bonds\n",x,y, pos);
      _vec_bonds_check = false;
    }
    return;
  }
  uint8_t pixels = _buffer.at(pos);
  uint8_t pixel = 0xff;

	switch (x%4) {					            //2-bit grayscale dot
    	case 0: pixel = (pixels & 0x3F) | ((uint8_t)color << 6); break;	
    	case 1: pixel = (pixels & 0xCF) | ((uint8_t)color << 4); break;	
    	case 2: pixel = (pixels & 0xF3) | ((uint8_t)color << 2); break;	
    	case 3: pixel = (pixels & 0xFC) | (uint8_t)color; break;		
	}
  buffer_it = _buffer.begin()+pos;
  *(buffer_it) = pixel;
  
}
