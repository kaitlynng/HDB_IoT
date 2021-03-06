#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#define LOGGER_ID       "test"
#define MACHINE_ID      "TestMachine"
#define CONTRACT_NAME   "TestContract"

// network credentials
#define NETWORK_SSID    "AIDrivers SG - 2.4G" // "Ng-fam-weefee-extraordinaire" // "AndroidAPf2b2" // "HDB4" //"mobilelogger@95";//"AIDrivers SG - 2.4G";//"@zfzenryu";//AIDrivers SG - 5G//SINGTEL-776B//AndroidAPf2b2//mobilelogger95//SPSS_AQC601//HDB2//HDB3//TLC
#define NETWORK_PASS    "@1Drivers75" // "tecknam67941827" // "rfkv0877" // "hdbdatalogger" //"logger@95";//"@1Drivers75";//"Zengfu9595";//@1Drivers75//eequuughah//rfkv0877//logger95//SPSS_AQC601//hdbdatalogger//TLC68620769

const int LOCAL_IP_ADDRESS[] = {192, 168, 10, 184}; 
const int GATEWAY_ADDRESS[] = {192, 168, 10, 1}; // need to configure!
const int SUBNET_ADDRESS[] = {255, 255, 255, 0};

const int PRIMARY_DNS[] = {192, 168, 10, 1};
const int SECONDARY_DNS[] = {0, 0, 0, 0};

// MQTT server
#define MQTT_ENDPOINT    "a132xxzx3351g7-ats.iot.ap-southeast-1.amazonaws.com"
#define MQTT_PORT        8883
#define MQTT_CLIENT      LOGGER_ID
#define MQTT_TOPIC       "$aws/things/" LOGGER_ID "/data"

// SQL credentials
#define MYSQL_HOSTNAME  "aidrivers-hdb-01.c08vyzk0x9qg.us-east-2.rds.amazonaws.com"
#define MYSQL_PORT      3306

#define MYSQL_USER      "admin"
#define MYSQL_PASS      "a1Drivers75"
#define DEFAULT_DB      "piling_dev_01"
#define DEFAULT_TABLE   "piling"

// NTP server
#define NTP_SERVER      "asia.pool.ntp.org"

// email details
#define EMAIL_SENDER_NAME       "HDB datalogger"
#define EMAIL_SENDER_ACCOUNT    "ahbang9956@gmail.com" //Kahwee@aidrivers.ai//ahbang9956@gmail.com//dummyai123@outlook.com
#define EMAIL_SENDER_PASS       "A@123456789" //radianceX3//A@123456789

#define SMTP_SERVER             "smtp.gmail.com" //smtp.office365.com//smtp.gmail.com
#define SMTP_SERVER_PORT        587 // 465 //465//587

#define EMAIL_NUM_RECIPIENTS    2
const char* EMAIL_RECIPIENTS[] = {"kirabananana@gmail.com", "nguyen2001ag2@gmail.com"};

#define UPDATE_RATE     10 //seconds

#define SQL_FLAG        true
#define MQTT_FLAG       true
#define EMAIL_FLAG      true

#define DEBUG_FLAG      true

#endif //USER_CONFIG_H_