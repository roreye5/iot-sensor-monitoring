#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>

/*
 * Connect the BMP180 sensor to pins 21 and 22 of the ESP32, SCL to 22, SDA to 21.
 */
class PressureSensor {
    private:
        Adafruit_BMP085 bmp;
    
    public:
    PressureSensor() {
        Serial.println("in pressure sensor constructor need to take away");
        if(!bmp.begin()) {
            Serial.println("[Pressure Sensor] Could not find a valid BMP085 sensor, check wiring!");
            while(1);
        }
    }

    /*
     * Returns current pressure reading in hPa.
     */
    float getPressure() {
        float temp = bmp.readPressure();
        return temp / 100.0;
    }

    /*
     * Returns current temperature reading in Farenheit.
     */
    float getTemperature() {
        float temp = bmp.readTemperature();
        return ((temp * 9 / 5) + 32);
    }
};