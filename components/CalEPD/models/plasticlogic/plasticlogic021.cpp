#include "plasticlogic021.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Constructor
PlasticLogic021::PlasticLogic021(EpdSpi2Cs& dio): 
  Adafruit_GFX(PLOGIC021_WIDTH, PLOGIC021_HEIGHT),
  PlasticLogic(PLOGIC021_WIDTH, PLOGIC021_HEIGHT, dio), IO(dio)
{
  printf("PlasticLogic021() %d*%d _buffer:%d\n",
  PLOGIC021_WIDTH, PLOGIC021_HEIGHT, PLOGIC021_BUFFER_SIZE);
}

//Initialize the display
void PlasticLogic021::init(bool debug)
{
    debug_enabled = debug;
    if (debug_enabled) {
      printf("PlasticLogic021::init(%d) bufferSize: %d width: %d height: %d\n", 
    debug, PLOGIC021_BUFFER_SIZE, PLOGIC021_WIDTH, PLOGIC021_HEIGHT);
    }
    initIO(debug);
    
    uint8_t size = getEPDsize();
    printf("EPD size ID: %d\n", size);

    if (size != 21) {
      ESP_LOGE(TAG, "ATTENTION the size responded by the display: %d does not mach this class (21)", size);
    }
    _setSize(size);
    _wakeUp();

    //printf("Epaper temperature after wakeUp: %d °C\n", IO.readTemp());
    //Set landscape mode: 1 as default
    setEpdRotation(1);
}

void PlasticLogic021::clearScreen(){
  for (uint16_t x = 0; x < sizeof(_buffer); x++)
  {
    _buffer[x] = 0xff;
  }
}

int PlasticLogic021::_getPixel(int x, int y) {
  // Not sure if width / height is correct since we handle rotation differently
    if ((x < 0) || (x >= PLOGIC021_WIDTH) || (y < 0) || (y >= PLOGIC021_HEIGHT)) return 5;  

	uint16_t byteIndex = x/4 + (y) * _nextline;
    switch (x%4) {
    		case 0: return ((unsigned int)(_buffer[byteIndex] & 0xC0) >> 6); 
     		case 1: return ((unsigned int)(_buffer[byteIndex] & 0x30) >> 4);
    		case 2: return ((unsigned int)(_buffer[byteIndex] & 0x0C) >> 2);
    		case 3: return ((unsigned int)(_buffer[byteIndex] & 0x03)); 
	}
  // Should not return 0, otherwise gives error: control reaches end of non-void function
  return 0;
}

/**
 * Not tested in my implementation
 */
void PlasticLogic021::scrambleBuffer() {
    for (int y=0; y<PLOGIC021_HEIGHT; y++) {                   // for each gateline...
        for (int x=0; x<PLOGIC021_WIDTH/2; x++) {             // for each sourceline...
            drawPixel(PLOGIC021_WIDTH-1-x, y, _getPixel(x,y+1));
            drawPixel(x, y, _getPixel(x+(PLOGIC021_WIDTH/2),y+1));
        }
    }
}

void PlasticLogic021::update(uint8_t updateMode)
{
  // Research how to send more data via SPI this way
  // E (452) spi_master: check_trans_valid(669): txdata transfer > host maximum
  ESP_LOGD(TAG, "Sending %d bytes buffer", sizeof(_buffer));
  
  scrambleBuffer(); 
  
  uint8_t pixelAccessPos[3] = {EPD_PIXELACESSPOS, 0, 0}; // In original class are -1 but that does not seem a valid SPI byte
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
        case 1:
          displayEngine[1] = 0x07;
          IO.data(programMtp, sizeof(programMtp));
          IO.data(displayEngine, sizeof(displayEngine));
          _waitBusy("EPD_UPD_FULL", EPD_TMG_LNG);
            break;
        case 2:
          programMtp[1] = 0x02;
          displayEngine[1] = 0x07;

          IO.data(programMtp, sizeof(programMtp));
          IO.data(displayEngine, sizeof(displayEngine));
          _waitBusy("EPD_UPD_FULL", EPD_TMG_LNG);
          break;
    }
    
  _powerOff();
}

void PlasticLogic021::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // check rotation, move pixel around if necessary
  switch (getRotation())
  {
    case 1:
      swap(x, y);
      x = PLOGIC021_WIDTH - x - 1;
      break;
    case 2:
      x = PLOGIC021_WIDTH - x - 1;
      y = PLOGIC021_HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = PLOGIC021_HEIGHT - y - 1;
      break;
  }
  
  uint8_t pixels = _buffer[x/4 + (y) * _nextline];
	switch (x%4) {					            //2-bit grayscale dot
    	case 0: _buffer[x/4 + (y) * _nextline] = (pixels & 0x3F) | ((uint8_t)color << 6); break;	
    	case 1: _buffer[x/4 + (y) * _nextline] = (pixels & 0xCF) | ((uint8_t)color << 4); break;	
    	case 2: _buffer[x/4 + (y) * _nextline] = (pixels & 0xF3) | ((uint8_t)color << 2); break;	
    	case 3: _buffer[x/4 + (y) * _nextline] = (pixels & 0xFC) | (uint8_t)color; break;		
	}
}
