#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

#define LOGGER_ID       "test"
#define MACHINE_ID      "AIDTestRig"

// network credentials
#define NETWORK_SSID    "AIDrivers SG - 2.4G" // "Ng-fam-weefee-extraordinaire" // "AndroidAPf2b2" // "HDB4" //"mobilelogger@95";//"AIDrivers SG - 2.4G";//"@zfzenryu";//AIDrivers SG - 5G//SINGTEL-776B//AndroidAPf2b2//mobilelogger95//SPSS_AQC601//HDB2//HDB3//TLC
#define NETWORK_PASS    "@1Drivers75" // "tecknam67941827" // "rfkv0877" // "hdbdatalogger" //"logger@95";//"@1Drivers75";//"Zengfu9595";//@1Drivers75//eequuughah//rfkv0877//logger95//SPSS_AQC601//hdbdatalogger//TLC68620769

const int LOCAL_IP_ADDRESS[] = {192, 168, 10, 184}; 
const int GATEWAY_ADDRESS[] = {192, 168, 10, 1}; // need to configure!
const int SUBNET_ADDRESS[] = {255, 255, 255, 0};

const int PRIMARY_DNS[] = {192, 168, 10, 1};
const int SECONDARY_DNS[] = {0, 0, 0, 0};

// HTTP server
#define HTTP_ENDPOINT   "http://hdb.aidrivers.ai/api/crane/" MACHINE_ID

#define UPDATE_RATE     10 //seconds
#define DEBUG_FLAG      true

#endif //USER_CONFIG_H_