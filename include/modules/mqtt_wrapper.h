#ifndef MQTT_WRAPPER_H_
#define MQTT_WRAPPER_H_

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>

class MqttWrapper
{
public:
    MqttWrapper(const char* server_name, int server_port, const char* client_name, 
                int timeout = 1, bool flag = true)
    : mqtt_client(800), m_server_name(server_name), m_server_port(server_port), m_client_name(client_name), 
      m_timeout(timeout), active_flag(flag) {}

    MqttWrapper() = delete;
    ~MqttWrapper() {}

    void set_certificates(char* aws_cert_ca, char* aws_cert_crt, char* aws_cert_private) {
        m_aws_cert_ca = aws_cert_ca;
        m_aws_cert_crt = aws_cert_crt;
        m_aws_cert_private = aws_cert_private;
    }

    void setup() {
        if (!active_flag) {
            Serial.println("WARNING: MQTT is inactivated!");
            return;
        }
        net.setCACert(m_aws_cert_ca);
        net.setCertificate(m_aws_cert_crt);
        net.setPrivateKey(m_aws_cert_private);
        mqtt_client.begin(m_server_name, m_server_port, net);
        connect();
    }

    // Reconnect to MQTT broker, the function has a blocking mechanism of 5 seconds
    
    void connect() {
        if (!active_flag) {
            return;
        }
        uint32_t stamp = millis();
        // Loop until we're reconnected, timeout

        Serial.print("Attempting MQTT connection...");

        while(!mqtt_client.connect(m_client_name)) {
            if (m_timeout > 0 && millis() - stamp > m_timeout * 1000) {
                Serial.println("WARNING: MQTT connection time out.");
                return;
            }
            Serial.print(".");
            delay(100);
        }

        if (mqtt_client.connected()) {
            Serial.println("MQTT server connected!");
        }
    }

    void publish(const char* topic, char* data) {
        if (!active_flag) {
            return;
        }

        if (!mqtt_client.connected()) {
            connect();
        }

        if (mqtt_client.publish(topic, data)) {
            Serial.println("Message published");
        }
        else {
            Serial.println("WARNING: Message publish failed");
        }

        mqtt_client.loop();
    }

private:

    MQTTClient mqtt_client;
    WiFiClientSecure net;
    
    const char* m_server_name;
    int m_server_port;
    const char* m_client_name;

    char* m_aws_cert_ca;
    char* m_aws_cert_crt;
    char* m_aws_cert_private;

    int m_timeout; // seconds, 0: do not timeout

    bool active_flag;
};

#endif
