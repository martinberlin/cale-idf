#include "FT6X36.h"

FT6X36 *FT6X36::_instance = nullptr;

FT6X36::FT6X36(int8_t intPin)
{
	_instance = this;

	i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = CONFIG_TOUCH_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = CONFIG_TOUCH_SDL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = CONFIG_I2C_MASTER_FREQUENCY;
    i2c_param_config(I2C_NUM_0, &conf);
    esp_err_t i2c_driver = i2c_driver_install(I2C_NUM_0, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	if (i2c_driver == ESP_OK) {
		printf("i2c_driver started correctly\n");
	} else {
		printf("i2c_driver error: %d\n", i2c_driver);
	}
	_intPin = intPin;
}

// Destructor should detach interrupt to the pin
FT6X36::~FT6X36()
{
	gpio_isr_handler_remove((gpio_num_t)CONFIG_TOUCH_INT);
}

void IRAM_ATTR FT6X36::isr(void* arg)
{
	ets_printf("ISR ");
	if (_instance)
		_instance->onInterrupt();
}

bool FT6X36::begin(uint8_t threshold)
{
	uint8_t data_panel_id;
	readRegister8(FT6X36_REG_PANEL_ID, &data_panel_id);

	if (data_panel_id != FT6X36_VENDID) {
		printf("FT6X36_VENDID does not match. Received:0x%x Expected:0x%x\n",data_panel_id,FT6X36_VENDID);
		return false;
		}
		printf("FT6X36_PANEL_ID: %x\n",data_panel_id);

	uint8_t chip_id;
	readRegister8(FT6X36_REG_CHIPID, &chip_id);
	if (chip_id != FT6206_CHIPID && chip_id != FT6236_CHIPID && chip_id != FT6336_CHIPID) {
		printf("FT6206_CHIPID does not match. Received:0x%x\n",chip_id);
		return false;
	}
	printf("FT6206_CHIPID: %x\n",chip_id);

	//gpio_set_direction((gpio_num_t)CONFIG_TOUCH_INT, GPIO_MODE_INPUT);
    //gpio_set_pull_mode((gpio_num_t)CONFIG_TOUCH_INT, GPIO_PULLUP_ONLY);

    // Sensor that goes HIGH when detects something (Ex. triggering a new image request in an exposition)
    gpio_config_t io_conf;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = 1ULL<<CONFIG_TOUCH_INT;  
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)1;
    gpio_config(&io_conf);

	esp_err_t isr_service = gpio_install_isr_service(0);
    printf("ISR trigger install response: 0x%x\n", isr_service);
	// Correct this: 
    gpio_isr_handler_add((gpio_num_t)CONFIG_TOUCH_INT, isr, (void*) 1);

	//attachInterrupt(digitalPinToInterrupt(_intPin), FT6X36::isr, FALLING);
	writeRegister8(FT6X36_REG_DEVICE_MODE, 0x00);
	writeRegister8(FT6X36_REG_THRESHHOLD, threshold);
	writeRegister8(FT6X36_REG_TOUCHRATE_ACTIVE, 0x0E);
	return true;
}

void FT6X36::registerIsrHandler(void (*fn)())
{
	_isrHandler = fn;
}

void FT6X36::registerTouchHandler(void (*fn)(TPoint point, TEvent e))
{
	_touchHandler = fn;
}

uint8_t FT6X36::touched()
{
	uint8_t data_buf;
    esp_err_t ret = readRegister8(FT6X36_REG_NUM_TOUCHES, &data_buf);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error reading from device: %s", esp_err_to_name(ret));
	 }

	if (data_buf > 2)
	{
		data_buf = 0;
	}

	return data_buf;
}

void FT6X36::loop()
{
	while (_isrCounter > 0)
	{
		_isrCounter--;
		processTouch();
	}
}

void FT6X36::processTouch()
{
	readData();
	uint8_t n = 0;
	TRawEvent event = (TRawEvent)_touchEvent[n];
	TPoint point{_touchX[n], _touchY[n]};

	if (event == TRawEvent::PressDown)
	{
		_points[0] = point;
		_pointIdx = 1;
		_dragMode = false;
		_touchStartTime = esp_timer_get_time();
		fireEvent(point, TEvent::TouchStart);
	}
	else if (event == TRawEvent::Contact)
	{
		if (_pointIdx < 10)
		{
			_points[_pointIdx] = point;
			_pointIdx += 1;
		}
		if (!_dragMode && _points[0].aboutEqual(point) && esp_timer_get_time() - _touchStartTime > 300)
		{
			_dragMode = true;
			fireEvent(point, TEvent::DragStart);
		}
		else if (_dragMode)
			fireEvent(point, TEvent::DragMove);

		fireEvent(point, TEvent::TouchMove);
	}
	else if (event == TRawEvent::LiftUp)
	{
		_points[9] = point;
		_touchEndTime = esp_timer_get_time();
		fireEvent(point, TEvent::TouchEnd);
		if (_dragMode)
		{
			fireEvent(point, TEvent::DragEnd);
			_dragMode = false;
		}
		if (_points[0].aboutEqual(point) && _touchEndTime - _touchStartTime <= 300)
		{
			fireEvent(point, TEvent::Tap);
			_points[0] = {0, 0};
			_touchStartTime = 0;
		}
	}
	else
	{
	}
}

void FT6X36::onInterrupt()
{
	_isrCounter++;

	if (_isrHandler)
	{
		_isrHandler();
	}
}

void FT6X36::readData(void)
{
	const uint8_t size = 16;
	uint8_t data[size];
	
	for (uint8_t i = 0; i < size; i++)
		//data[i] = Wire.read();
		data[i] = 0;
#if defined(CONFIG_FT6X36_DEBUG) && CONFIG_FT6X36_DEBUG==1
	printf("REGISTERS:\n");
	for (int16_t i = 0; i < size; i++)
	{
		printf("0x%x=0x%x\n", i, data[i]);
	}
	printf("TOUCHES: %d  GESTURE: %d\n", data[FT6X36_REG_NUM_TOUCHES], data[FT6X36_REG_GESTURE_ID]);
#endif
	_touches = data[FT6X36_REG_NUM_TOUCHES];

	const uint8_t addrShift = 6;
	for (uint8_t i = 0; i < 2; i++)
	{
		_touchX[i] = data[FT6X36_REG_P1_XH + i * addrShift] & 0x0F;
		_touchX[i] <<= 8;
		_touchX[i] |= data[FT6X36_REG_P1_XL + i * addrShift];
		_touchY[i] = data[FT6X36_REG_P1_YH + i * addrShift] & 0x0F;
		_touchY[i] <<= 8;
		_touchY[i] |= data[FT6X36_REG_P1_YL + i * addrShift];
		_touchEvent[i] = data[FT6X36_REG_P1_XH + i * addrShift] >> 6;
	}
}

void FT6X36::writeRegister8(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, FT6X36_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg , ACK_CHECK_EN);
	i2c_master_write_byte(cmd, value , ACK_CHECK_EN);
	i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
	// Needs to be replaced by ESP-IDF I2C master
	/* 
	_wire->beginTransmission(FT6X36_ADDR);
	_wire->write(reg);
	_wire->write(value);
	_wire->endTransmission();
	*/
}

uint8_t FT6X36::readRegister8(uint8_t reg, uint8_t *data_buf)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, FT6X36_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
	// Research: Why it's started a 2nd time here
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FT6X36_ADDR << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(cmd, data_buf, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
	//FT6X36_REG_GESTURE_ID

	// Needs to be replaced by ESP-IDF I2C master
	/* 
	_wire->beginTransmission(FT6X36_ADDR);
	_wire->write(reg);
	_wire->endTransmission();

	_wire->requestFrom((uint8_t)FT6X36_ADDR, (uint8_t)1);
	uint8_t value = _wire->read();
	*/
#if defined(CONFIG_TOUCH_I2C_DEBUG) && CONFIG_TOUCH_I2C_DEBUG==1
	printf("REG 0x%x: 0x%x\n",reg,ret);
#endif

	return ret;
}

void FT6X36::fireEvent(TPoint point, TEvent e)
{
	if (_touchHandler)
		_touchHandler(point, e);
}

#ifdef FT6X36_DEBUG
/* 
void FT6X36::debugInfo()
{
	Serial.print("TH_DIFF: ");
	Serial.println(readRegister8(FT6X36_REG_FILTER_COEF));
	Serial.print("CTRL: ");
	Serial.println(readRegister8(FT6X36_REG_CTRL));
	Serial.print("TIMEENTERMONITOR: ");
	Serial.println(readRegister8(FT6X36_REG_TIME_ENTER_MONITOR));
	Serial.print("PERIODACTIVE: ");
	Serial.println(readRegister8(FT6X36_REG_TOUCHRATE_ACTIVE));
	Serial.print("PERIODMONITOR: ");
	Serial.println(readRegister8(FT6X36_REG_TOUCHRATE_MONITOR));
	Serial.print("RADIAN_VALUE: ");
	Serial.println(readRegister8(FT6X36_REG_RADIAN_VALUE));
	Serial.print("OFFSET_LEFT_RIGHT: ");
	Serial.println(readRegister8(FT6X36_REG_OFFSET_LEFT_RIGHT));
	Serial.print("OFFSET_UP_DOWN: ");
	Serial.println(readRegister8(FT6X36_REG_OFFSET_UP_DOWN));
	Serial.print("DISTANCE_LEFT_RIGHT: ");
	Serial.println(readRegister8(FT6X36_REG_DISTANCE_LEFT_RIGHT));
	Serial.print("DISTANCE_UP_DOWN: ");
	Serial.println(readRegister8(FT6X36_REG_DISTANCE_UP_DOWN));
	Serial.print("DISTANCE_ZOOM: ");
	Serial.println(readRegister8(FT6X36_REG_DISTANCE_ZOOM));
	Serial.print("CIPHER: ");
	Serial.println(readRegister8(FT6X36_REG_CHIPID));
	Serial.print("G_MODE: ");
	Serial.println(readRegister8(FT6X36_REG_INTERRUPT_MODE));
	Serial.print("PWR_MODE: ");
	Serial.println(readRegister8(FT6X36_REG_POWER_MODE));
	Serial.print("FIRMID: ");
	Serial.println(readRegister8(FT6X36_REG_FIRMWARE_VERSION));
	Serial.print("FOCALTECH_ID: ");
	Serial.println(readRegister8(FT6X36_REG_PANEL_ID));
	Serial.print("STATE: ");
	Serial.println(readRegister8(FT6X36_REG_STATE)); 
	*/
}
#endif