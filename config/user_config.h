#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#include <Arduino.h>

#define CONTRACT_NAME   "TENGAHPAC07/D/027"

// network credentials
#define NETWORK_SSID    "HDB4"
#define NETWORK_PASS    "hdbdatalogger"

const int LOCAL_IP_ADDRESS[] = {192, 168, 0, 184}; 
const int GATEWAY_ADDRESS[] = {192, 168, 0, 1}; // need to configure!
const int SUBNET_ADDRESS[] = {255, 255, 255, 0};

const int PRIMARY_DNS[] = {192, 168, 0, 1};
const int SECONDARY_DNS[] = {0, 0, 0, 0};

// MQTT server
#define MQTT_SERVER         "3.tcp.ngrok.io"  // "192.168.10.110" // "6.tcp.ngrok.io" // "2.tcp.ngrok.io"
#define MQTT_SERVER_PORT     21369 // 1883 // 18010  // 11576
#define MQTT_CLIENT         "ESP32Client"
#define MQTT_TOPIC          "Crane/1"

// SQL credentials
#define MYSQL_USER      "leo"
#define MYSQL_PASS      "lqNZ3K]2U0)br5"
#define DEFAULT_DB      "leo_1"
#define DEFAULT_TABLE   "Test4"
const int MYSQL_SERVER_ADDRESS[] = {216, 219, 81, 80}; // IP of the MySQL *server* here// to check daily  //216,219,81,80//52,76,27,242

#define NTP_SERVER      "asia.pool.ntp.org"


// email details
#define EMAIL_SENDER_NAME       "HDB datalogger"
#define EMAIL_SENDER_ACCOUNT    "ahbang9956@gmail.com" //Kahwee@aidrivers.ai//ahbang9956@gmail.com//dummyai123@outlook.com
#define EMAIL_SENDER_PASS       "A@123456789" //radianceX3//A@123456789

#define SMTP_SERVER             "smtp.gmail.com" //smtp.office365.com//smtp.gmail.com
#define SMTP_SERVER_PORT        587 // 465 //465//587

// edit the num_recipients to reflect the number of email recipients
//#define EMAIL_NUM_RECIPIENTS    3
//const String EMAIL_RECIPIENTS[EMAIL_NUM_RECIPIENTS] = {"aidrivers.crane@gmail.com", "radiancekw@gmail.com", "tankangjie@gmail.com"}; //aidrivers.demo@gmail.com

#define EMAIL_NUM_RECIPIENTS    2
const char* EMAIL_RECIPIENTS[] = {"kirabananana@gmail.com", "AMKN2C32A_PileDataLogger@hdb.gov.sg"}; //aidrivers.demo@gmail.com

#define UPDATE_RATE     10 //seconds

#define SQL_FLAG        true
#define MQTT_FLAG       true
#define EMAIL_FLAG      true

#define DEBUG_FLAG      true

#endif //USER_CONFIG_H_
