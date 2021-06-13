#ifndef GPS_WRAPPER_H_
#define GPS_WRAPPER_H_

#include <Arduino.h>
#include <TinyGPS++.h>

class GpsWrapper
{
public:
    GpsWrapper(const int rx_pin, const int tx_pin, const int baud = 9600) 
    : m_rx_pin(rx_pin), m_tx_pin(tx_pin), m_baud(baud) {}

    GpsWrapper() = delete;
    ~GpsWrapper() {};

    void setup() {} // Do nothing as Serial2 is initialized outside

    bool poll(double data[2]) {
        if (Serial2.available() > 0) {
            if (gps.encode(Serial2.read()) && gps.location.isValid()) {
                data[0] = gps.location.lat();
                data[1] = gps.location.lng();
                return true;
            }
            return false;
        }
        
        if (millis() > 5000 && gps.charsProcessed() < 10) {
            // Serial.println("WARNING: No GPS detected!");
        }

        return false;
    }

private:
    int m_rx_pin;
    int m_tx_pin;
    int m_baud;

    TinyGPSPlus gps;
};

#endif