#include <stdint.h>
#include <cstdlib>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
//#define I2C_DEBUG 1 - renamed to TOUCH_I2C_DEBUG
//#define FT6X36_DEBUG 1

#define FT6X36_ADDR						0x38

#define FT6X36_REG_DEVICE_MODE			0x00
#define FT6X36_REG_GESTURE_ID			0x01
#define FT6X36_REG_NUM_TOUCHES			0x02
#define FT6X36_REG_P1_XH				0x03
#define FT6X36_REG_P1_XL				0x04
#define FT6X36_REG_P1_YH				0x05
#define FT6X36_REG_P1_YL				0x06
#define FT6X36_REG_P1_WEIGHT			0x07
#define FT6X36_REG_P1_MISC				0x08
#define FT6X36_REG_P2_XH				0x09
#define FT6X36_REG_P2_XL				0x0A
#define FT6X36_REG_P2_YH				0x0B
#define FT6X36_REG_P2_YL				0x0C
#define FT6X36_REG_P2_WEIGHT			0x0D
#define FT6X36_REG_P2_MISC				0x0E
#define FT6X36_REG_THRESHHOLD			0x80
#define FT6X36_REG_FILTER_COEF			0x85
#define FT6X36_REG_CTRL					0x86
#define FT6X36_REG_TIME_ENTER_MONITOR	0x87
#define FT6X36_REG_TOUCHRATE_ACTIVE		0x88
#define FT6X36_REG_TOUCHRATE_MONITOR	0x89 // value in ms
#define FT6X36_REG_RADIAN_VALUE			0x91
#define FT6X36_REG_OFFSET_LEFT_RIGHT	0x92
#define FT6X36_REG_OFFSET_UP_DOWN		0x93
#define FT6X36_REG_DISTANCE_LEFT_RIGHT	0x94
#define FT6X36_REG_DISTANCE_UP_DOWN		0x95
#define FT6X36_REG_DISTANCE_ZOOM		0x96
#define FT6X36_REG_LIB_VERSION_H		0xA1
#define FT6X36_REG_LIB_VERSION_L		0xA2
#define FT6X36_REG_CHIPID				0xA3
#define FT6X36_REG_INTERRUPT_MODE		0xA4
#define FT6X36_REG_POWER_MODE			0xA5
#define FT6X36_REG_FIRMWARE_VERSION		0xA6
#define FT6X36_REG_PANEL_ID				0xA8
#define FT6X36_REG_STATE				0xBC

#define FT6X36_PMODE_ACTIVE				0x00
#define FT6X36_PMODE_MONITOR			0x01
#define FT6X36_PMODE_STANDBY			0x02
#define FT6X36_PMODE_HIBERNATE			0x03

#define FT6X36_VENDID					0x11
#define FT6206_CHIPID					0x06
#define FT6236_CHIPID					0x36
#define FT6336_CHIPID					0x64

#define FT6X36_DEFAULT_THRESHOLD		22

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

	bool aboutEqual(const TPoint point)
	{
		const uint8_t maxDeviation = 5;
		return abs(x - point.x) <= maxDeviation && abs(y - point.y) <= maxDeviation;
	}
};

class FT6X36
{
	static void isr();
public:
    // TwoWire * wire will be replaced by ESP-IDF https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
	FT6X36(int8_t intPin);
	~FT6X36();
	bool begin(uint8_t threshold = FT6X36_DEFAULT_THRESHOLD);
	void registerIsrHandler(void(*fn)());
	void registerTouchHandler(void(*fn)(TPoint point, TEvent e));
	uint8_t touched();
	void loop();
	void processTouch();
	void debugInfo();

private:
	void onInterrupt();
	void readData(void);
	void writeRegister8(uint8_t reg, uint8_t val);
	uint8_t readRegister8(uint8_t reg);
	void fireEvent(TPoint point, TEvent e);

	static FT6X36 * _instance;
	//TwoWire * _wire = nullptr;
	uint8_t _intPin;

	void(*_isrHandler)() = nullptr;
	void(*_touchHandler)(TPoint point, TEvent e) = nullptr;
	volatile uint8_t _isrCounter = 0;
	
	uint8_t _touches;
	uint16_t _touchX[2], _touchY[2], _touchEvent[2];
	TPoint _points[10];
	uint8_t _pointIdx = 0;
	unsigned long _touchStartTime = 0;
	unsigned long _touchEndTime = 0;
	bool _dragMode = false;
};

