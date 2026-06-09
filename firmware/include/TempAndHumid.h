/**
 * @file TempAndHumid.h
 * @brief This file contains the declaration of the TempAndHumid class.
 */

#include <Arduino.h>
#include <DHT.h>

/**
 * @class TempAndHumid
 * @brief A class that represents a temperature and humidity sensor.
 * 
 * You can use this class to read the temperature and humidity from a DHT sensor.
 */
class TempAndHumid {
private:
    int sensorPin; /**< The pin number of the sensor. */
    int sensorType; /**< The type of the sensor. */
    static constexpr int sampleSize = 10; /**< The size of the sample array. */
    int readings[sampleSize]; /**< An array to store the sensor readings. */
    int readIndex = 0; /**< The index of the next reading to be stored in the array. */
    float total = 0; /**< The sum of all the sensor readings. */
    DHT* dht; /**< A pointer to the DHT sensor object. */

public:
    /**
     * @brief Constructs a new TempAndHumid object.
     * @param dhtPin The pin number of the DHT sensor.
     * @param dhtType The type of the DHT sensor.
     */
    TempAndHumid(int dhtPin, int dhtType) : sensorPin(dhtPin), sensorType(dhtType) {
        dht = new DHT(dhtPin, dhtType);
        dht->begin();
    }

    /**
     * @brief Destroys the TempAndHumid object and frees the memory.
     */
    ~TempAndHumid() {
        delete dht;
    }
    
    /**
     * @brief Gets the temperature reading from the sensor.
     * @return The temperature in degrees Fahrenheit.
     */
    float getTemp() {
        return dht->readTemperature(true); // Read temperature in Fahrenheit
    }

    /**
     * @brief Gets the humidity reading from the sensor.
     * @return The relative humidity in percentage.
     */
    float getHumidity() {
        return dht->readHumidity();
    }
};