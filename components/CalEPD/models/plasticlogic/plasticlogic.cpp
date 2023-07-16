#include "plasticlogic.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/task.h"

// Plasticlogic will replace Epd as baseclass for this models and have all common methods for all EPDs of this manufacturer
void PlasticLogic::initIO(bool debug) {
  IO.init(4, debug); // 4MHz frequency
  
  if (CONFIG_EINK_RST > -1) {
    IO.reset(5);
  } else {
    IO.cmd(EPD_SOFTWARERESET);
  }
}

uint8_t PlasticLogic::getEPDsize() {
  int8_t response = 0;
  uint8_t programMtp[2] = {EPD_PROGRAMMTP, 0x02};
  uint8_t setMtpAddress[3] = {EPD_MTPADDRESSSETTING, 0xF2, 0x04};
  
  IO.data(programMtp, sizeof(programMtp));
  _waitBusy("programMtp");
  IO.data(setMtpAddress, sizeof(setMtpAddress));
  _waitBusy("setMtpAddress");

  uint8_t reg = 0x43|EPD_REGREAD;
  uint8_t regRead[2] = {reg, 0xFF};

  // Read 1 dummy bytes
  IO.readRegister(regRead,sizeof(regRead));
  response = IO.readRegister(regRead,sizeof(regRead));

  ESP_LOGI(TAG, "MISO responded with: %d", response);

  switch (response)
  {
  case 49:
    response = IO.readRegister(regRead,sizeof(regRead));
    
    if (response==49)    {
      return 11; // 1.1" detected
    } else {
      return 14; // 1.4" detected
    }        
    break;

  case 50:
    return 21;
    break;
  case 51:
    return 31;
    break;
  default:
    response = 0;
    break;
  }
  _setSize(response);
  return response;
}

uint8_t PlasticLogic::readTemperature()
{
  return IO.readTemp();
}

std::string PlasticLogic::readTemperatureString(char type) {
  uint8_t temperature = readTemperature();
  std::string temp = std::to_string(temperature);
  switch (type)
  {
  case 'c':
    temp = temp + " °C";
    break;
  case 'f':
  uint8_t fahrenheit = temperature * 9/5 + 32;
    temp = std::to_string(fahrenheit);
    temp += " °F";
    break;
  }
  return temp;
}

/**
 * Sets internal EPD into portrait or Landscape mode
 * Not sure if it will be a public method at the end of integration
 */
void PlasticLogic::setEpdRotation(uint8_t o) {
   uint8_t settingDataEntryPortrait[2] = {EPD_DATENTRYMODE, 0x02}; // All of them
   uint8_t settingDataEntryLandscape[2] = {EPD_DATENTRYMODE, 0x20};
    switch (size) {
      case 11:
        settingDataEntryLandscape[1] = 0x07;
        break;
      case 14:
        settingDataEntryLandscape[1] = 0x02;
        break;
    }

   switch (o)
   {
   case 1:
     /* Landscape mode */
     IO.data(settingDataEntryLandscape, sizeof(settingDataEntryLandscape));
     break;
   
   case 2:
     /* Portrait */
     IO.data(settingDataEntryPortrait, sizeof(settingDataEntryPortrait));
     break;
   }
}

void PlasticLogic::_setSize(uint8_t epdSize) {
  size = epdSize;
}

void PlasticLogic::_wakeUp(){
  uint8_t panelSetting[2] = {EPD_PANELSETTING, 0x12};
  uint8_t settingWriteRectangular[5] = {EPD_WRITEPXRECTSET, 0x00, 0x47, 0x00, 0x93};
  uint8_t settingVcom[5] = {EPD_VCOMCONFIG, 0x0, 0x0, 0x24, 0x07};
  ESP_LOGI(TAG, "Setting wakeup EPD_WRITEPXRECTSET for size:%d\n",size);

  switch (size) {
        // case 11: Defaults
        case 14:
          // EPD_WRITEPXRECTSET, 0, 0xB3, 0x3C, 0x9F
        settingWriteRectangular[2] = 0xB3;
        settingWriteRectangular[3] = 0x3C;
        settingWriteRectangular[4] = 0x9F;
          break;
        case 21:
        panelSetting[1] = 0x10;
          // EPD_WRITEPXRECTSET, 0, 0xEF, 0, 145->91(hexa)
        settingWriteRectangular[2] = 0xEF;
        settingWriteRectangular[4] = 0x91;
          break;
        case 31:
          // EPD_WRITEPXRECTSET, 0, 0x97, 0, 0x9b
        settingWriteRectangular[2] = 0x97;
        settingWriteRectangular[4] = 0x9b;
        break;
  }

    // 1. First 3 set differently depending on size
    IO.data(panelSetting, sizeof(panelSetting));
    _waitBusy("Panel setting");
    // 2.
    IO.data(settingWriteRectangular, sizeof(settingWriteRectangular));
    _waitBusy("settingWriteRectangular");
    // 3.
    IO.data(settingVcom, sizeof(settingVcom));
    _waitBusy("VCOM configuration");

    uint8_t settingDriverVoltage[3] = {EPD_DRIVERVOLTAGE, 0x25, 0xff};
    IO.data(settingDriverVoltage, sizeof(settingDriverVoltage));
    _waitBusy("Driver voltage setting");

    uint8_t settingVborder[2] = {EPD_BORDERSETTING, 0x04};
    IO.data(settingVborder, sizeof(settingVborder));
    _waitBusy("Vborder setting");

    uint8_t settingLoadMono[2] = {EPD_LOADMONOWF, 0x60};
    IO.data(settingLoadMono, sizeof(settingLoadMono));
    _waitBusy("Load mono");

    uint8_t settingTemperature[2] = {EPD_INTTEMPERATURE, 0x0a};
    IO.data(settingTemperature, sizeof(settingTemperature));
    _waitBusy("Temperature");

    uint8_t settingBoost[3] = {EPD_BOOSTSETTING, 0x22, 0x17};
    IO.data(settingBoost, sizeof(settingBoost));
    _waitBusy("Boost setting");
}

void PlasticLogic::_powerOn(void) {
  uint8_t setResolution[5]   = {EPD_SETRESOLUTION, 0x00, 0xEF, 0x00, 0x93};

  switch (size)
  {
  case 14:
  case 21:
  case 31:
    setResolution[4]   = 0x9F;
    break;
  }
  
  uint8_t setTcomTiming[3]   = {EPD_TCOMTIMING   , 0x67, 0x65};
  uint8_t setPowerSeq[4]     = {EPD_POWERSEQUENCE, 0x00, 0x00, 0x00};
  uint8_t setPowerControl[2] = {EPD_POWERCONTROL , 0xD1};

  IO.data(setResolution, sizeof(setResolution));
  _waitBusy("Panel resolution");

  IO.data(setTcomTiming, sizeof(setTcomTiming));
  _waitBusy("TCOM Timing");
  
  IO.data(setPowerSeq, sizeof(setPowerSeq));
  _waitBusy("Power sequence");

  IO.data(setPowerControl, sizeof(setPowerControl));
  _waitBusy("setPowerControl");
  // Resolve the wait for pump read
  //vTaskDelay(140/portTICK_PERIOD_MS);       // Only because reading the value below is not working
  uint8_t reg = 0x15|EPD_REGREAD;
  uint8_t regRead[2] = {reg, 0xFF};

  while (IO.readRegister(regRead,sizeof(regRead)) == 0) {}   // Wait until Internal Pump is ready 
}

void PlasticLogic::_waitBusy(const char* message, uint16_t busy_time){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();
  // On high is busy
  if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) {
  while (1){
    if (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 1) break;
    vTaskDelay(1);
    if (esp_timer_get_time()-time_since_boot>7000000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
  } else {
    vTaskDelay(busy_time/portTICK_PERIOD_MS); 
  }
}

void PlasticLogic::_waitBusy(const char* message){
  if (debug_enabled) {
    ESP_LOGI(TAG, "_waitBusy for %s", message);
  }
  int64_t time_since_boot = esp_timer_get_time();

  while (gpio_get_level((gpio_num_t)CONFIG_EINK_BUSY) == 0){
    vTaskDelay(1/portTICK_PERIOD_MS); 

    if (esp_timer_get_time()-time_since_boot>100000)
    {
      if (debug_enabled) ESP_LOGI(TAG, "Busy Timeout");
      break;
    }
  }
}

// Called _poweroff in microEPD
void PlasticLogic::_powerOff(){
  uint8_t setPowerControl[2] = {EPD_POWERCONTROL , 0xD0};
  IO.data(setPowerControl, sizeof(setPowerControl)); 
  _waitBusy("power_off");
  
  setPowerControl[1] = 0xC0;
  IO.data(setPowerControl, sizeof(setPowerControl));
  _waitBusy("setPowerControl");
}

/**
 * Putting the UC8156 in deep sleep mode with less than 1µA current @3.3V
 * Reset pin toggling needed to wakeup the driver IC again.
 */ 
void PlasticLogic::_sleep(){
  printf("Deepsleep called. In order to wake up from sleep mode this display needs to be initialized again calling init() method\n");
  uint8_t deepsleep[5] = {0x21 , 0xff, 0xff, 0xff, 0xff};
  IO.data(deepsleep, sizeof(deepsleep));
}

// GFX functions
// display.print / println handling .TODO: Implement printf
size_t PlasticLogic::write(uint8_t v){
  Adafruit_GFX::write(v);
  return 1;
}
uint8_t PlasticLogic::_unicodeEasy(uint8_t c) {
  if (c<191 && c>131 && c!=176) { // 176 is °W 
    c+=64;
  }
  return c;
}

void PlasticLogic::print(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
}

void PlasticLogic::println(const std::string& text){
   for(auto c : text) {
     if (c==195 || c==194) continue; // Skip to next letter

     // _unicodeEasy will just sum 64 and get the right character when using umlauts and other characters:
     c = _unicodeEasy(c);
     write(uint8_t(c));
   }
   write(10); // newline
}

void PlasticLogic::newline() {
  write(10);
}
