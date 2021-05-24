#ifndef MQTT_WRAPPER_H_
#define MQTT_WRAPPER_H_

#include <Arduino.h>

#include <PubSubClient.h>
#include <WiFi.h>

class MqttWrapper
{
public:
    MqttWrapper(WiFiClient &client, const char* server_name, int server_port, const char* client_name,  
                int timeout = 1, bool flag = true)
    : m_server_port(server_port), m_timeout(timeout), active_flag(flag) {
        if (!active_flag) {
            Serial.println("WARNING: MQTT is inactivated!");
        }
        m_server_name = strdup(server_name); // convert const char* to char* for API calls
        m_client_name = strdup(client_name);

        mqtt_client = PubSubClient(client);
    }

    MqttWrapper() = delete;
    ~MqttWrapper() {}

    void setup() {
        if (!active_flag) {
            return;
        }
        mqtt_client.setServer(m_server_name, m_server_port);
        //mqtt_client.setCallback(callback);  
    }

    // Reconnect to MQTT broker, the function has a blocking mechanism of 5 seconds
    
    void connect() {
        if (!active_flag) {
            return;
        }
        uint32_t stamp = millis();
        // Loop until we're reconnected, timeout

        Serial.print("Attempting MQTT connection...");
        while(!mqtt_client.connected()) {
            if (m_timeout > 0 && millis() - stamp > m_timeout * 1000) {
                Serial.println("WARNING: MQTT connection timedout.");
                return;
            }
            //Attempt to connect
            if (mqtt_client.connect(m_client_name)) {
                Serial.println("MQTT server Connected");            }
        }
    }

    // Callback funciton hasn't been proper define, please define it at your own use
    // void callback(char* topic, byte* message, unsigned int length) {
    //     /* To be implemented */
    //     if (!active_flag) {
    //         return;
    //     }

    //     Serial.print("Message arrived on topic: ");
    //     Serial.print(topic);
    //     Serial.print(". Message: ");
    //     String messageTemp;

    //     for (int i = 0; i < length; i++) {
    //         Serial.print((char)message[i]);
    //         messageTemp += (char)message[i];
    //     }
    //     Serial.println();

    //     // Feel free to add more if statements to control more GPIOs with MQTT

    //     // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
    //     // Changes the output state according to the message
    //     if (String(topic) == m_topic_name) 
    //     {
    //         Serial.print("Changing output to ");
    //         if (messageTemp == "on") 
    //         {
    //             Serial.println("on");
    //         }
    //         else if (messageTemp == "off")
    //         {
    //             Serial.println("off");
    //         }
    //     }
    // }

    void publish(const char* topic, char* data) {
        if (!active_flag) {
            return;
        }
        if (mqtt_client.publish(topic, data)) {
            Serial.println("Message published");
        }
        else{
            Serial.println("WARNING: Message publish failed");
        }
    }

    void subscribe(char* topic) {
        if (!active_flag) {
            return;
        }
        if (mqtt_client.subscribe(topic)) {
            Serial.print("Subscribed to topic: ") ;
            Serial.println(topic);
        }
        else{
            Serial.println("WARNING: Failed to subscribe to topic");
        }
    }

private:
    char* m_server_name;
    int m_server_port;
    char* m_client_name;

    int m_timeout; // seconds, 0: do not timeout

    WiFiClient client;
    PubSubClient mqtt_client;

    bool active_flag = true;
};

#endif
