#ifndef GPS_WRAPPER_H_
#define GPS_WRAPPER_H_

#include <Arduino.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

class GpsWrapper
{
public:
    GpsWrapper(const int tx_pin, const int rx_pin, const int baud = 9600) 
    : m_tx_pin(tx_pin), m_rx_pin(rx_pin), m_baud(baud), ss(rx_pin, tx_pin) {}

    GpsWrapper() = delete;
    ~GpsWrapper() {};

    void setup() { ss.begin(m_baud); }

    bool poll(double data[2]) {
        if (ss.available() > 0) {
            //Serial.println("Reading GPS data...");
            if (gps.encode(ss.read()) && gps.location.isValid()) {
                data[0] = gps.location.lat();
                data[1] = gps.location.lng();

                return true;
            }
        }
        
        if (millis() > 5000 && gps.charsProcessed() < 10) {
            Serial.println("WARNING: No GPS detected!");
        }

        return false;
    }

private:
    int m_tx_pin;
    int m_rx_pin;
    int m_baud;

    TinyGPSPlus gps;
    SoftwareSerial ss;
};

#endif