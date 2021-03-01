// This is the first experimental Touch component for the LILYGO EPD47 touch overlay
// Controller: L58  -> https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/files/6059098/L58.V1.0.pdf
// Note: Rotation is only working for certain angles (0 works alright, 2 also) Others still need to be corrected
#include "L58Touch.h"

L58Touch *L58Touch::_instance = nullptr;
static const char *TAG = "i2c-touch";

//Handle indicating I2C is ready to read the touch
SemaphoreHandle_t TouchSemaphore = xSemaphoreCreateBinary();

L58Touch::L58Touch(int8_t intPin)
{
	_instance = this;

	i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)CONFIG_TOUCH_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)CONFIG_TOUCH_SDL;
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
L58Touch::~L58Touch()
{
	gpio_isr_handler_remove((gpio_num_t)CONFIG_TOUCH_INT);
}

bool L58Touch::begin(uint8_t threshold, uint16_t width, uint16_t height)
{
	_touch_width = width;
	_touch_height = height;
	if (width == 0 || height ==0) {
		ESP_LOGE(TAG,"begin(uint8_t threshold, uint16_t width, uint16_t height) did not receive the width / height so touch cannot be rotation aware");
	}

    // INT pin triggers the callback function on the Falling edge of the GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = 1ULL<<CONFIG_TOUCH_INT;  
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = (gpio_pulldown_t) 0; // disable pull-down mode
    io_conf.pull_up_en   = (gpio_pullup_t) 1;   // pull-up mode
    gpio_config(&io_conf);

	esp_err_t isr_service = gpio_install_isr_service(0);
    printf("ISR trigger install response: 0x%x %s\n", isr_service, (isr_service==0)?"ESP_OK":"");
    gpio_isr_handler_add((gpio_num_t)CONFIG_TOUCH_INT, isr, (void*) 1);

	// Wake up: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/blob/master/src/touch.cpp#L123
	uint8_t buf[2] = {0xD1, 0X06};
	writeData(buf, sizeof(buf));
	return true;
}

void L58Touch::registerTouchHandler(void (*fn)(TPoint point, TEvent e))
{
	_touchHandler = fn;
	if (CONFIG_FT6X36_DEBUG) printf("Touch handler function registered\n");
}

uint8_t L58Touch::touched()
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

void L58Touch::loop()
{
	processTouch();
}

void IRAM_ATTR L58Touch::isr(void* arg)
{
	/* Un-block the interrupt processing task now */
    xSemaphoreGive(TouchSemaphore);
	//xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void L58Touch::processTouch()
{
	/* Task move to Block state to wait for interrupt event */
	if (xSemaphoreTake(TouchSemaphore, portMAX_DELAY) == false) return;
	TPoint point = scanPoint();
    
    // This should be measured in LiftUp event (Events not documented)
    _touchEndTime = esp_timer_get_time()/1000;

    // Very primitive way to fire a Tap event without press & liftup
    // Essentially avoids fast repetition but is not real Tap
    if ( _touchEndTime - _touchStartTime <= 15) {
	  fireEvent(point, TEvent::Tap);
    }
    
}

uint8_t L58Touch::read8(uint8_t regName) {
	uint8_t buf;
	readRegister8(regName, &buf);
	return buf;
}

TPoint L58Touch::scanPoint()
{
    _touchStartTime = esp_timer_get_time()/1000;
	TPoint point{0,0};
	uint8_t pointIdx = 0;
    uint8_t buffer[40] = {0};
    uint32_t sumL = 0, sumH = 0;

    buffer[0] = 0xD0;
    buffer[1] = 0x00;
    readBytes(buffer, 7);

    if (buffer[0] == 0xAB) {
        clearFlags();
        return point;
    }

    pointIdx = buffer[5] & 0xF;

    if (pointIdx == 1) {
        buffer[5] = 0xD0;
        buffer[6] = 0x07;
        readBytes( &buffer[5], 2);
        sumL = buffer[5] << 8 | buffer [6];

    } else if (pointIdx > 1) {
        buffer[5] = 0xD0;
        buffer[6] = 0x07;
        readBytes( &buffer[5], 5 * (pointIdx - 1) + 3);
        sumL = buffer[5 * pointIdx + 1] << 8 | buffer[5 * pointIdx + 2];
    }
    clearFlags();

    for (int i = 0 ; i < 5 * pointIdx; ++i) {
        sumH += buffer[i];
    }

    if (sumH != sumL) {
        pointIdx = 0;
    }
    if (pointIdx) {
        uint8_t offset;
        for (int i = 0; i < pointIdx; ++i) {
            if (i == 0) {
                offset = 0;
            } else {
                offset = 4;
            }
            data[i].id =  (buffer[i * 5 + offset] >> 4) & 0x0F;
            data[i].state = buffer[i * 5 + offset] & 0x0F;
            if (data[i].state == 0x06) {
                data[i].state = 0x07;
            } else {
                data[i].state = 0x06;
            }
            data[i].y = (uint16_t)((buffer[i * 5 + 1 + offset] << 4) | ((buffer[i * 5 + 3 + offset] >> 4) & 0x0F));
            data[i].x = (uint16_t)((buffer[i * 5 + 2 + offset] << 4) | (buffer[i * 5 + 3 + offset] & 0x0F));

            printf("X[%d]:%d Y:%d\n", i, data[i].x, data[i].y);
        }

    } else {
        pointIdx = 1;
        data[0].id = (buffer[0] >> 4) & 0x0F;
        data[0].state = 0x06;
        data[0].y = (uint16_t)((buffer[0 * 5 + 1] << 4) | ((buffer[0 * 5 + 3] >> 4) & 0x0F));
        data[0].x = (uint16_t)((buffer[0 * 5 + 2] << 4) | (buffer[0 * 5 + 3] & 0x0F));
        printf("X:%d Y:%d\n", data[0].x, data[0].y);
	}
     
	uint16_t x = data[0].x;
	uint16_t y = data[0].y;

	// Make touch rotation aware
	switch (_rotation)
  {
	// 0- no rotation: Works OK inverting Y axis
	case 0:
	    y = _touch_height - y;
		break;

	case 1:
	    swap(x, y);
		y = _touch_width - y;
        x = _touch_height - x;
		break;

	case 2: // Works OK
		x = _touch_width - x;
		break;

	case 3:
		swap(x, y);
		break;
  }

	point = {x, y};
    return point;
	
}

void L58Touch::writeRegister8(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, FT6X36_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg , ACK_CHECK_EN);
	i2c_master_write_byte(cmd, value , ACK_CHECK_EN);
	i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

void L58Touch::writeData(uint8_t *data, int len)
{
    if (len==0) return; 
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write(cmd, data, len, ACK_CHECK_EN);
	i2c_master_stop(cmd);
}

uint8_t L58Touch::readRegister8(uint8_t reg, uint8_t *data_buf)
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

	
	//FT6X36_REG_GESTURE_ID. Check if it can be read!
#if defined(FT6X36_DEBUG) && FT6X36_DEBUG==1
	printf("REG 0x%x: 0x%x\n",reg,ret);
#endif

	return ret;
}

void L58Touch::readBytes(uint8_t *data, int len) {
	if (len==0) return; 
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, FT6X36_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write(cmd, data, 2, ACK_CHECK_EN);

	i2c_master_start(cmd);
	
	i2c_master_write_byte(cmd, FT6X36_ADDR << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
	i2c_master_read(cmd, data, len, (i2c_ack_type_t) ACK_VAL);
	i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

	if (ret == ESP_OK) {
        for (int i = 0; i < len; i++) {
            printf("0x%02x ", data[i]);
            if ((i + 1) % 16 == 0) {
                printf("\r\n");
            }
        }
        if (len % 16) {
            printf("\r\n");
        }
    } else if (ret == ESP_ERR_TIMEOUT) {
		// Getting a lot of this!
        //ESP_LOGW(TAG, "Bus is busy");
    } else {
        ESP_LOGW(TAG, "Read failed");
    }
}

void L58Touch::fireEvent(TPoint point, TEvent e)
{
	if (_touchHandler)
		_touchHandler(point, e);
}

void L58Touch::setRotation(uint8_t rotation) {
	_rotation = rotation;
}

void L58Touch::setTouchWidth(uint16_t width) {
	printf("touch w:%d\n",width);
	_touch_width = width;
}

void L58Touch::setTouchHeight(uint16_t height) {
	printf("touch h:%d\n",height);
	_touch_height = height;
}

void L58Touch::clearFlags() {
	uint8_t buf[3] = {0xD0, 0X00, 0XAB};
	writeData(buf, sizeof(buf));
}

void L58Touch::sleep() {
	uint8_t buf[2] = {0xD1, 0X05};
	writeData(buf, sizeof(buf));
}
