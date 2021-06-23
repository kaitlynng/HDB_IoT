#define TINY_GSM_MODEM_SIM7600

#define GSM_BAUD        115200
#define GSM_RXPIN       16
#define GSM_TXPIN       17

#define GSM_TIMEOUT     30 // seconds 

#include <TinyGSMClient.h>
#include <Arduino.h>

#define SerialAT Serial2


#define TINY_GSM_USE_GPRS       true
#define TINY_GSM_USE_WIFI       false

class GsmWrapper
{
public:
    GsmWrapper(const int rx_pin, const int tx_pin, const int baud = 115200) 
    : m_rx_pin(rx_pin), m_tx_pin(tx_pin), m_baud(baud) {}

    GsmWrapper() = delete;
    ~GsmWrapper() {};

    void setup()
    {
        SerialAT.begin(m_baud, SERIAL_8N1, m_rx_pin, m_tx_pin); 
        Serial.println(F("Initializing modem..."));
        modem.init();
    }

private:
    int m_rx_pin;
    int m_tx_pin;
    int m_baud;

    TinyGsm modem(Serial2);

}

