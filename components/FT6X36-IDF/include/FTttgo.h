// This is the first experimental Touch component for the LILYGO EPD47 touch overlay
// Controller is unknown, no tech specs are available, so far I could only implement as an ESP-IDF component the original example:
// https://github.com/Xinyuan-LilyGO/LilyGo-EPD47
// More info about this epaper: 
// https://github.com/martinberlin/cale-idf/wiki/Model-parallel-ED047TC1.h
#include <stdint.h>
#include <cstdlib>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#ifndef touch_ttgo_h
#define touch_ttgo_h
// I2C Constants
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */


#define FT6X36_ADDR						0x5A

#define FT6X36_REG_GESTURE_ID			0x01
#define FT6X36_REG_NUM_TOUCHES			0x02
#define FT6X36_DEFAULT_THRESHOLD		22

// From: https://github.com/lvgl/lv_port_esp32/blob/master/components/lvgl_esp32_drivers/lvgl_touch/ft6x36.h
#define FT6X36_MSB_MASK                 0x0F
#define FT6X36_LSB_MASK                 0xFF

enum class TRawEvent
{
	PressDown,
	LiftUp,
	Contact,
	NoEvent
};

enum class TEvent
{
	None,
	TouchStart,
	TouchMove,
	TouchEnd,
	Tap,
	DragStart,
	DragMove,
	DragEnd
};

struct TPoint
{
	uint16_t x;
	uint16_t y;
	/**
	 * This is being used in the original library but I'm not using it in this implementation
	 */
	bool aboutEqual(const TPoint point)
	{
		return abs(x - point.x) <= 5 && abs(y - point.y) <= 5;
	}
};



class FTttgo
{
	static void IRAM_ATTR isr(void* arg);
	typedef struct {
        uint8_t id;
        uint8_t state;
        uint16_t x;
        uint16_t y;
    } TouchData_t;

public:
    // TwoWire * wire will be replaced by ESP-IDF https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
	FTttgo(int8_t intPin);
	~FTttgo();
	bool begin(uint8_t threshold = FT6X36_DEFAULT_THRESHOLD, uint16_t width = 0, uint16_t height = 0);
	void registerTouchHandler(void(*fn)(TPoint point, TEvent e));
	uint8_t touched();
	void loop();
	void processTouch();
	// Helper functions to make the touch display aware
	void setRotation(uint8_t rotation);
	void setTouchWidth(uint16_t width);
	void setTouchHeight(uint16_t height);
	// Pending implementation. How much x->touch y↓touch is placed (In case is smaller than display)
	void setXoffset(uint16_t x_offset);
	void setYoffset(uint16_t y_offset);
	void sleep();
	// Smart template from EPD to swap x,y:
    template <typename T> static inline void
    swap(T& a, T& b)
    {
      T t = a;
      a = b;
      b = t;
    }
	void(*_touchHandler)(TPoint point, TEvent e) = nullptr;

	TouchData_t data[5];
	
private:
	TPoint scanPoint();
	void writeRegister8(uint8_t reg, uint8_t val);
	void writeData(uint8_t *data, int len);
	void readBytes(uint8_t *data, int len);
	uint8_t readRegister8(uint8_t reg, uint8_t *data_buf);
	void fireEvent(TPoint point, TEvent e);
	uint8_t read8(uint8_t regName);
	void clearFlags();

	static FTttgo * _instance;
	uint8_t _intPin;
	
	// Make touch rotation aware:
	uint8_t _rotation = 0;
	uint16_t _touch_width = 0;
	uint16_t _touch_height = 0;

	uint8_t _touches;
	uint16_t _touchX[2], _touchY[2], _touchEvent[2];
	TPoint _points[10];
	uint8_t _pointIdx = 0;
	unsigned long _touchStartTime = 0;
	unsigned long _touchEndTime = 0;
    uint8_t lastEvent = 3; // No event
	uint16_t lastX = 0;
	uint16_t lastY = 0;
	bool _dragMode = false;
	const uint8_t maxDeviation = 5;
};

#endif