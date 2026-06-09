#include <Wire.h>
#include <U8x8lib.h>


/*************************************************
 * This is the class to handle the display of the data on the OLED screen.
 * 
 * The display is initialized in the constructor and then the displayData function is called to update the display with the latest sensor readings.
 * The displayData function takes in the temperature and humidity values and displays them on the OLED screen.
 *************************************************/

 class Display {
public:
    Display() : _oled(U8X8_PIN_NONE) {}

    void begin() {
        _oled.begin();
        _oled.setFont(u8x8_font_chroma48medium8_r);
        _oled.clearDisplay();
    }

    void showMessage(const String& msg) {
        _oled.clear();
        _oled.draw1x2String(0, 0, msg.c_str());
    }

    void showSensorData(float temp, float humidity){
        _oled.clear();
        String tempStr = "Temp: " + String(temp, 1) + " F";
        String humidStr = "Humidity: " + String(humidity, 1) + " %";
        _oled.draw1x2String(0, 0, tempStr.c_str());
        _oled.draw1x2String(0, 2, humidStr.c_str());
    }

private:
    U8X8_SSD1306_128X32_UNIVISION_HW_I2C _oled;
};