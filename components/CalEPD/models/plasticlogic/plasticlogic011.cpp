#include "plasticlogic011.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
PlasticLogic011::PlasticLogic011(EpdSpi2Cs& dio): 
  Adafruit_GFX(PLOGIC011_WIDTH, PLOGIC011_HEIGHT),
  PlasticLogic(PLOGIC011_WIDTH, PLOGIC011_HEIGHT, dio), IO(dio)
{
  printf("PlasticLogic011() %d*%d\n",
  PLOGIC011_WIDTH, PLOGIC011_HEIGHT);
}

// Destructor
PlasticLogic011::~PlasticLogic011() {
  // Disconnect from SPI
  IO.release();
}

//Initialize the display
void PlasticLogic011::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) {
      printf("PlasticLogic011::init(%d) bufferSize: %d width: %d height: %d\n", 
    debug, (int)PLOGIC011_BUFFER_SIZE, (int)PLOGIC011_WIDTH, (int)PLOGIC011_HEIGHT);
    }
    initIO(debug);
    
    uint8_t size = getEPDsize();
    _setSize(size);
    printf("EPD size ID: %d\n", size);

    if (size != 11) {
      ESP_LOGE(TAG, "ATTENTION the size responded by the display: %d does not mach this class", size);
    }

    _wakeUp();

    //printf("Epaper temperature after wakeUp: %d °C\n", IO.readTemp());
    //Set landscape mode as default
    setEpdRotation(1);
}

void PlasticLogic011::clearScreen(){
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = 0xff;
  }
}

void PlasticLogic011::update(uint8_t updateMode)
{
  ESP_LOGD(TAG, "Sending %d bytes buffer", sizeof(_buffer));

  uint8_t pixelAccessPos[3] = {EPD_PIXELACESSPOS, 0x00, 0x93};
  uint8_t programMtp[2] = {EPD_PROGRAMMTP, 0x00};
  uint8_t displayEngine[2] = {EPD_DISPLAYENGINE, 0x03};

  IO.data(pixelAccessPos, sizeof(pixelAccessPos));

  bufferEpd[0] = 0x10;
  // Copy GFX buffer contents:
  for (int i=1; i < sizeof(bufferEpd); i++) {
    bufferEpd[i] = _buffer[i-1];
  }

  IO.data(bufferEpd,sizeof(bufferEpd));
  
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

void PlasticLogic011::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = PLOGIC011_WIDTH - x - 1;
      break;
    case 2:
      x = PLOGIC011_WIDTH - x - 1;
      y = PLOGIC011_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = PLOGIC011_HEIGHT - y - 1;
      break;
  }
  
  y=y+3;
  uint8_t pixels = _buffer[x/4 + (y) * _nextline];
	switch (x%4) {					            //2-bit grayscale dot
    	case 0: _buffer[x/4 + (y) * _nextline] = (pixels & 0x3F) | ((uint8_t)color << 6); break;	
    	case 1: _buffer[x/4 + (y) * _nextline] = (pixels & 0xCF) | ((uint8_t)color << 4); break;	
    	case 2: _buffer[x/4 + (y) * _nextline] = (pixels & 0xF3) | ((uint8_t)color << 2); break;	
    	case 3: _buffer[x/4 + (y) * _nextline] = (pixels & 0xFC) | (uint8_t)color; break;		
	}
}
