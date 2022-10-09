// This is the first experimental Touch component for the LILYGO EPD47 touch overlay
// Controller: L58  -> https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/files/6059098/L58.V1.0.pdf
// Note: Rotation is only working for certain angles (0 works alright, 2 also) Others still need to be corrected
#include "L58Touch.h"

L58Touch *L58Touch::_instance = nullptr;
static const char *TAG = "i2c-touch";

// Handle indicating I2C is ready to read the touch
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

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
    // !< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here.
    conf.clk_flags = 0;
#endif

    i2c_param_config(I2C_NUM_0, &conf);
    esp_err_t i2c_driver = i2c_driver_install(I2C_NUM_0, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (i2c_driver == ESP_OK)
    {
        printf("i2c_driver started correctly\n");
    }
    else
    {
        printf("i2c_driver error: %d\n", i2c_driver);
    }
    _intPin = intPin;
}

// Destructor should detach interrupt to the pin
L58Touch::~L58Touch()
{
    gpio_isr_handler_remove((gpio_num_t)CONFIG_TOUCH_INT);
}

bool L58Touch::begin(uint16_t width, uint16_t height)
{
    _touch_width = width;
    _touch_height = height;
    if (width == 0 || height == 0)
    {
        ESP_LOGE(TAG, "begin(uint8_t threshold, uint16_t width, uint16_t height) did not receive the width / height so touch cannot be rotation aware");
    }

    // INT pin triggers the callback function on the Falling edge of the GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = 1ULL << CONFIG_TOUCH_INT;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = (gpio_pulldown_t)0; // disable pull-down mode
    io_conf.pull_up_en = (gpio_pullup_t)1;     // pull-up mode
    gpio_config(&io_conf);

    esp_err_t isr_service = gpio_install_isr_service(0);
    printf("ISR trigger install response: 0x%x %s\n", isr_service, (isr_service == 0) ? "ESP_OK" : "");
    gpio_isr_handler_add((gpio_num_t)CONFIG_TOUCH_INT, isr, (void *)1);

    // Wake up: https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/blob/master/src/touch.cpp#L123
    uint8_t buf[2] = {0xD1, 0X06};
    writeData(buf, sizeof(buf));
    return true;
}

void L58Touch::registerTouchHandler(void (*fn)(TPoint point, TEvent e))
{
    _touchHandler = fn;
    if (CONFIG_FT6X36_DEBUG)
        printf("Touch handler function registered\n");
}

void L58Touch::registerMultiTouchHandler(void (*fn)(TPoint point1, TPoint point2, TEvent e))
{
    _multiTouchHandler = fn;
    if (CONFIG_FT6X36_DEBUG)
        printf("Multi-touch handler function registered\n");
}

void L58Touch::loop()
{
    processTouch();
}

void IRAM_ATTR L58Touch::isr(void *arg)
{
    /* Un-block the interrupt processing task now */
    xSemaphoreGive(TouchSemaphore);
}

void L58Touch::processTouch()
{
    /* Task move to Block state to wait for interrupt event */
    if (xSemaphoreTake(TouchSemaphore, portMAX_DELAY) == false)
        return;

#if defined(CONFIG_L58_MULTITOUCH) && CONFIG_L58_MULTITOUCH == 1
    std::pair<TPoint, TPoint> points;
    points = scanMultiPoint();
    TPoint point0 = points.first;
    TPoint point1 = points.second;
    if (tapDetectionEnabled && _touchEndTime - _touchStartTime <= tapDetectionMillisDiff)
    {
        fireMultiTouch(point0, point1, TEvent::Tap);
    }
    else
    {
        fireMultiTouch(point0, point1, TEvent::None);
    }
#else
    TPoint point0 = scanPoint();
    if (!tapDetectionEnabled)
    {
        fireEvent(point0, TEvent::Tap);
    }
    if (tapDetectionEnabled && _touchEndTime - _touchStartTime <= tapDetectionMillisDiff)
    {
        fireEvent(point0, TEvent::Tap);
    }
#endif
}

uint8_t L58Touch::read8(uint8_t regName)
{
    uint8_t buf;
    readRegister8(regName, &buf);
    return buf;
}

TPoint L58Touch::scanPoint()
{
    TPoint point{0, 0, 0};
    uint8_t pointIdx = 0;
    uint8_t buffer[40] = {0};

    buffer[0] = 0xD0;
    buffer[1] = 0x00;
    readBytes(buffer, 7);

    if (buffer[0] == 0xAB)
    {
        clearFlags();
        return point;
    }

    pointIdx = buffer[5] & 0xF;

    if (pointIdx == 1)
    {
        buffer[5] = 0xD0;
        buffer[6] = 0x07;
        readBytes(&buffer[5], 2);
    }
    else if (pointIdx > 1)
    {
        buffer[5] = 0xD0;
        buffer[6] = 0x07;
        readBytes(&buffer[5], 5 * (pointIdx - 1) + 3);
    }
    clearFlags();

    // Only one touch mode
    pointIdx = 1;
    data[0].id = (buffer[0] >> 4) & 0x0F;
    data[0].event = (buffer[0] & 0x0F) >> 1;
    data[0].y = (uint16_t)((buffer[0 * 5 + 1] << 4) | ((buffer[0 * 5 + 3] >> 4) & 0x0F));
    data[0].x = (uint16_t)((buffer[0 * 5 + 2] << 4) | (buffer[0 * 5 + 3] & 0x0F));
    if (data[0].event == 3)
    { /** Press */
        _touchStartTime = esp_timer_get_time() / 1000;
    }
    if (data[0].event == 0)
    { /** Lift up */
        _touchEndTime = esp_timer_get_time() / 1000;
    }
    printf("X:%d Y:%d E:%d\n", data[0].x, data[0].y, data[0].event);

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

    point = {x, y, data[0].event};
    return point;
}

/**
 * @brief Returns up to two points using https://en.cppreference.com/w/cpp/utility/pair/make_pair
 *
 * @return std::pair<TPoint, TPoint>
 */
std::pair<TPoint, TPoint> L58Touch::scanMultiPoint()
{
    TPoint point0{0, 0, 0};
    TPoint point1{0, 0, 0};
    uint8_t pointIdx = 0;
    uint8_t buffer[40] = {0};

    buffer[0] = 0xD0;
    buffer[1] = 0x00;
    readBytes(buffer, 7);

    if (buffer[0] == 0xAB)
    {
        clearFlags();
        return std::make_pair(point0, point1);
    }

    pointIdx = buffer[5] & 0xF;

    if (pointIdx == 1)
    {
        buffer[5] = 0xD0;
        buffer[6] = 0x07;
        readBytes(&buffer[5], 2);
    }
    else if (pointIdx > 1)
    {
        buffer[5] = 0xD0;
        buffer[6] = 0x07;
        readBytes(&buffer[5], 5 * (pointIdx - 1) + 3);
    }
    clearFlags();

    if (pointIdx)
    {

        for (int i = 0; i < 2; ++i)
        {
            data[i].id = (buffer[i * 5] >> 4) & 0x0F;
            data[i].event = buffer[i * 5] & 0x0F;
            data[i].y = (uint16_t)((buffer[i * 5 + 1] << 4) | ((buffer[i * 5 + 3] >> 4) & 0x0F));
            data[i].x = (uint16_t)((buffer[i * 5 + 2] << 4) | (buffer[i * 5 + 3] & 0x0F));
            printf("X[%d]:%d Y:%d E:%d\n", i, data[i].x, data[i].y, data[i].event);
            uint16_t x = data[i].x;
            uint16_t y = data[i].y;
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

            if (i == 0)
            {
                point0 = {x, y, data[0].event};
            }
            else
            {
                point1 = {x, y, data[0].event};
            }
        }
    }
    else
    {
        // Only one touch mode
        pointIdx = 1;
        data[0].id = (buffer[0] >> 4) & 0x0F;
        data[0].event = (buffer[0] & 0x0F) >> 1;
        data[0].y = (uint16_t)((buffer[0 * 5 + 1] << 4) | ((buffer[0 * 5 + 3] >> 4) & 0x0F));
        data[0].x = (uint16_t)((buffer[0 * 5 + 2] << 4) | (buffer[0 * 5 + 3] & 0x0F));
        if (data[0].event == 3)
        { /** Press */
            _touchStartTime = esp_timer_get_time() / 1000;
        }
        if (data[0].event == 0)
        { /** Lift up */
            _touchEndTime = esp_timer_get_time() / 1000;
        }
        printf("X:%d Y:%d E:%d\n", data[0].x, data[0].y, data[0].event);
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
        point0 = {x, y, data[0].event};
    }

    return std::make_pair(point0, point1);
}

void L58Touch::writeRegister8(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, L58_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, value, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

void L58Touch::writeData(uint8_t *data, int len)
{
    if (len == 0)
        return;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write(cmd, data, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
}

uint8_t L58Touch::readRegister8(uint8_t reg, uint8_t *data_buf)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, L58_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
    // Research: Why it's started a 2nd time here
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (L58_ADDR << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(cmd, data_buf, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

#if defined(FT6X36_DEBUG) && FT6X36_DEBUG == 1
    printf("REG 0x%x: 0x%x\n", reg, ret);
#endif

    return ret;
}

void L58Touch::readBytes(uint8_t *data, int len)
{
    if (len == 0)
        return;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, L58_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cmd, data, 2, ACK_CHECK_EN);

    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, L58_ADDR << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, data, len, (i2c_ack_type_t)ACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK)
    {
        for (int i = 0; i < len; i++)
        {
            printf("0x%02x ", data[i]);
            if ((i + 1) % 16 == 0)
            {
                printf("\r\n");
            }
        }
        if (len % 16)
        {
            printf("\r\n");
        }
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        // Getting a lot of this!
        // ESP_LOGW(TAG, "Bus is busy");
    }
    else
    {
        ESP_LOGW(TAG, "Read failed");
    }
}

void L58Touch::fireEvent(TPoint point, TEvent e)
{
    if (_touchHandler)
        _touchHandler(point, e);
}

void L58Touch::fireMultiTouch(TPoint point1, TPoint point2, TEvent e)
{
    if (_multiTouchHandler)
        _multiTouchHandler(point1, point2, e);
}

void L58Touch::setRotation(uint8_t rotation)
{
    _rotation = rotation;
}

void L58Touch::setTouchWidth(uint16_t width)
{
    printf("touch w:%d\n", width);
    _touch_width = width;
}

void L58Touch::setTouchHeight(uint16_t height)
{
    printf("touch h:%d\n", height);
    _touch_height = height;
}

void L58Touch::clearFlags()
{
    uint8_t buf[3] = {0xD0, 0X00, 0XAB};
    writeData(buf, sizeof(buf));
}

void L58Touch::sleep()
{
    uint8_t buf[2] = {0xD1, 0X05};
    writeData(buf, sizeof(buf));
}
