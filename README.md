# CALE ESP-IDF beta

This is the beginning, and a very raw try, to make CALE compile in the Espressif IOT Development Framework. At the moment to explore how difficult it can be to pass an existing ESP32 Arduino framework project to a ESP-IDF based one and to measure how much Framerate we can get compiling this with Espressif's own dev framework. 



## Benchmarking results 

On a Lolin 32 board are this:

    Speed test
    ----------

    digitalRead               : 0.145 us
    digitalWrite              : 0.110 us
    pinMode                   : 2.767 us
    multiply byte             : 0.000 us
    divide byte               : 0.000 us
    add byte                  : 0.000 us
    multiply integer          : 0.055 us
    divide integer            : 0.060 us
    add integer               : 0.055 us
    multiply long             : 0.000 us
    divide long               : 0.000 us
    add long                  : 0.000 us
    multiply float            : 0.000 us
    divide float              : 0.000 us
    add float                 : 0.000 us
    itoa()                    : 0.705 us
    ltoa()                    : 0.800 us
    dtostrf()                 : 11.450 us
    random()                  : 0.725 us
    y |= (1<<x)               : 0.046 us
    bitSet()                  : 0.046 us
    analogRead()              : 5.950 us
    digitalWrite() PWM         : 0.110 us
    delay(1)                  : 1000.000 us
    delay(100)                : 100000.000 us
    delayMicroseconds(2)      : 2.716 us
    delayMicroseconds(5)      : 5.918 us
    delayMicroseconds(100)    : 100.450 us
    -----------

### Compile this 

If you have ESP32S2BETA (The ones that Espressif sent before official release)

    idf.py -D IDF_TARGET=esp32s2beta menuconfig

If it's an ESP32S2

    idf.py -D IDF_TARGET=esp32s2beta menuconfig

And then just build and flash

    idf.py build
    idf.py flash

To open the serial monitor

    idf.py monitor

Please note that to exit the monitor in Espressif documentation says Ctrl+] but in Linux this key combination is:

    Ctrl+5