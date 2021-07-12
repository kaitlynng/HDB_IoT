#ifndef WIFI_WRAPPER_H_
#define WIFI_WRAPPER_H_

#include <Arduino.h>

class WiFiWrapper
{
public:
    WiFiWrapper(const char* network_ssid, const char* network_pass, int timeout = 10) 
    : m_network_ssid(network_ssid), m_network_pass(network_pass), m_timeout(timeout) {}

    WiFiWrapper() = delete;
    ~WiFiWrapper() {}

    void print_network_details() {
        Serial.print("Local IP: ");
        Serial.println(WiFi.localIP());

        Serial.print("Subnet Mask: ");
        Serial.println(WiFi.subnetMask());

        Serial.print("Gateway IP: ");
        Serial.println(WiFi.gatewayIP());

        Serial.print("DNS 1: ");
        Serial.println(WiFi.dnsIP(0));
        Serial.print("DNS 2: ");
        Serial.println(WiFi.dnsIP(1));
    }

    void set_static_ip_params(const int local_ip[4], const int gateway[4], const int subnet[4],
                              const int primary_dns[4], const int secondary_dns[4]) {
    
        m_local_ip = IPAddress(local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
        m_gateway = IPAddress(gateway[0], gateway[1], gateway[2], gateway[3]);
        m_subnet = IPAddress(subnet[0], subnet[1], subnet[2], subnet[3]);

        m_primary_dns = IPAddress(primary_dns[0], primary_dns[1], primary_dns[2], primary_dns[3]);
        m_secondary_dns = IPAddress(secondary_dns[0], secondary_dns[1], secondary_dns[2], secondary_dns[3]);
    }         

    int setup() {
        WiFi.mode(WIFI_STA);
        if (!WiFi.config(m_local_ip, m_gateway, m_subnet, m_primary_dns, m_secondary_dns)) {
            Serial.println("WARNING: STA Failed to configure");
        }
        return connect();
    }

    void connect_unblocking() {
        WiFi.begin(m_network_ssid, m_network_pass);
    }

    int connect() {

        unsigned long startTime = millis();

        Serial.printf("Connecting to %s ", m_network_ssid);
        WiFi.begin(m_network_ssid, m_network_pass);

        // try to connect every 0.1s for wifi connection timeout
        while (millis() - startTime < m_timeout * 1000) {
            if (WiFi.status() == WL_CONNECTED) break;
            delay(100);
            Serial.print(".");
        }

        switch (WiFi.status()) 
        {
            case 6 : Serial.print("Wifi not connected!"); return 0;
            case 3 : Serial.println("Wifi connected! Continue with ops"); return 1;
            case 1 : Serial.println("No wifi connected"); return 0; //ESP.restart(); break; //append_file(SD, "/error_log", "No wifi \n");
            default : Serial.print("Unknown code: "); return 0;
        }

    }

    void get_local_ip(char* cbuff) {
        IPAddress ip = WiFi.localIP();
        sprintf(cbuff, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    }

    void get_host_by_name(const char* hostname, IPAddress ip) {
        WiFi.hostByName(hostname, ip);
    }

    int get_wifi_status() {
        return WiFi.status();
    }

private:
    const char* m_network_ssid;
    const char* m_network_pass;

    IPAddress m_local_ip;
    IPAddress m_gateway;
    IPAddress m_subnet;
    IPAddress m_primary_dns;
    IPAddress m_secondary_dns;

    int m_timeout;
};

#endif