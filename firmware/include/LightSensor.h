#include <Arduino.h>
/**
 * @file LightSensor.h
 * @brief Contains the declaration of the LightSensor class.
 */



/**
 * @class LightSensor
 * @brief This class is used to read the light sensor data.
 * 
 * The light sensor data is read using the analogRead function and then converted to lux using a mapping function.
 * The lux value is then returned to the user.
 */
class LightSensor {
public:
  /**
   * @brief Construct a new Light Sensor object
   * 
   * @param pin The pin to which the light sensor is connected
   */
  LightSensor(int pin) : _sensorPin(pin) {
    analogReadResolution(12); // Ensure 12-bit ADC resolution is set
    for (int i = 0; i < _sampleSize; i++) _readings[i] = 0; // Initialize the _readings array
  }

  /**
   * @brief Get the Lux value
   * 
   * @return float The lux value
   */
  float getLux() {
    int rawValue = analogRead(_sensorPin);
    _addToFilter(rawValue); // Add the new reading to the filter
    return _calculateLux(_average); // Calculate lux based on the filtered _average
  }


private:
  static constexpr int _sampleSize = 10; // Size of the moving _average filter

  int _sensorPin;
  int _readings[_sampleSize]; // Array to store _readings
  int _readIndex = 0; // Current position in the array
  float _total = 0; // Total of the current _readings
  float _average = 0; // Average of the _readings

  
  /**
   * @brief Calculate the lux value based on the raw sensor value
   * 
   * @param rawValue The raw sensor value
   * @return float The calculated lux value
   */
  float _calculateLux(int rawValue) {
    if (rawValue > 1500) return 40; // Cap at min lux for values above 1500
    else if (rawValue >= 740) return map(rawValue, 1500, 740, 40, 160);
    else if (rawValue >= 300) return map(rawValue, 740, 300, 160, 1000);
    else if (rawValue > 0) return map(rawValue, 300, 0, 1000, 3300);
    else return 3300; // Assume max lux for values at 0
  }

  /**
   * @brief Add a new sensor reading to the moving average filter
   * 
   * @param newValue The new sensor reading
   */
  void _addToFilter(int newValue) {
    // Subtract the oldest reading from _total and replace it with the new one
    _total = _total - _readings[_readIndex] + newValue;
    _readings[_readIndex] = newValue;
    _readIndex = (_readIndex + 1) % _sampleSize; // Advance to the next position
    // Calculate the _average of the current window of _readings
    _average = _total / _sampleSize;
  }
};