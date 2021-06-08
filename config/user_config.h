#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#include <Arduino.h>

#define DEVICE_NAME     "HDB6"
#define CONTRACT_NAME   "TENGAHPAC07/D/027"

// network credentials
#define NETWORK_SSID    "Ng-fam-weefee-extraordinaire" // "AIDrivers SG - 2.4G" // "AndroidAPf2b2" // "HDB4" //"mobilelogger@95";//"AIDrivers SG - 2.4G";//"@zfzenryu";//AIDrivers SG - 5G//SINGTEL-776B//AndroidAPf2b2//mobilelogger95//SPSS_AQC601//HDB2//HDB3//TLC
#define NETWORK_PASS    "tecknam67941827" // "@1Drivers75" // "rfkv0877" // "hdbdatalogger" //"logger@95";//"@1Drivers75";//"Zengfu9595";//@1Drivers75//eequuughah//rfkv0877//logger95//SPSS_AQC601//hdbdatalogger//TLC68620769

const int LOCAL_IP_ADDRESS[] = {192, 168, 1, 184}; 
const int GATEWAY_ADDRESS[] = {192, 168, 1, 254}; // need to configure!
const int SUBNET_ADDRESS[] = {255, 255, 255, 0};

const int PRIMARY_DNS[] = {192, 168, 1, 254};
const int SECONDARY_DNS[] = {0, 0, 0, 0};

// MQTT server
#define MQTT_ENDPOINT    "a132xxzx3351g7-ats.iot.us-east-2.amazonaws.com"
#define MQTT_PORT        8883
#define MQTT_CLIENT      DEVICE_NAME
#define MQTT_TOPIC       "$aws/things/" DEVICE_NAME "/shadow/update"

// SQL credentials
#define MYSQL_USER      "leo"
#define MYSQL_PASS      "lqNZ3K]2U0)br5"
#define DEFAULT_DB      "leo_1"
#define DEFAULT_TABLE   "Test4"
const int MYSQL_SERVER_ADDRESS[] = {216, 219, 81, 80}; // IP of the MySQL *server* here// to check daily  //216,219,81,80//52,76,27,242

// NTP server
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
const char* EMAIL_RECIPIENTS[] = {"kirabananana@gmail.com", "nguyen2001ag2@gmail.com"}; //aidrivers.demo@gmail.com

#define UPDATE_RATE     10 //seconds

#define SQL_FLAG        true
#define MQTT_FLAG       true
#define EMAIL_FLAG      true

#define DEBUG_FLAG      true

#endif //USER_CONFIG_H_