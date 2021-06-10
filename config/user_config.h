#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#define LOGGER_ID       "CR020004"
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
#define MQTT_ENDPOINT    "a132xxzx3351g7-ats.iot.ap-southeast-1.amazonaws.com"
#define MQTT_PORT        8883
#define MQTT_CLIENT      LOGGER_ID
#define MQTT_TOPIC       "$aws/things/" LOGGER_ID "/data"

// SQL credentials
#define MYSQL_HOSTNAME  "aidrivers-hdb-01.c3yqyazj0wp1.us-east-2.rds.amazonaws.com"
#define MYSQL_PORT      3306

#define MYSQL_USER      "admin"
#define MYSQL_PASS      "a1Drivers75"
#define DEFAULT_DB      "leo_1"
#define DEFAULT_TABLE   "Test4"

// NTP server
#define NTP_SERVER      "asia.pool.ntp.org"

// email details
#define EMAIL_SENDER_NAME       "HDB datalogger"
#define EMAIL_SENDER_ACCOUNT    "ahbang9956@gmail.com" 
#define EMAIL_SENDER_PASS       "A@123456789" 

#define SMTP_SERVER             "smtp.gmail.com" 
#define SMTP_SERVER_PORT        587

#define EMAIL_NUM_RECIPIENTS    2
const char* EMAIL_RECIPIENTS[] = {"kirabananana@gmail.com", "TengahPAC7_PileDataLogger@hdb.gov.sg"};

#define UPDATE_RATE     10 //seconds

#define SQL_FLAG        true
#define MQTT_FLAG       true
#define EMAIL_FLAG      true

#define DEBUG_FLAG      false

#endif //USER_CONFIG_H_
