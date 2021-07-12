#ifndef SYSTEM_CONFIG_H_
#define SYSTEM_CONFIG_H_

// RTC params //
#define SD_CS                15
#define GMTOFFSET_SEC        28800 //GMTOFFSET * 60 * 60 //https://en.wikipedia.org/wiki/List_of_UTC_time_offsets REFER TO LINK FOR OFFSET NUMBER
#define DAYLIGHTOFFSET_SEC   0
#define RTC_TIMEOUT          10 // seconds

#define RTC_SYNC_RATE        30*60 // 30*60 // seconds

// WIFI network params //
#define NETWORK_TIMEOUT 10 // seconds

// CAN params //
#define CAN_RX_QUEUE_SIZE   40
#define CAN_RXPIN           GPIO_NUM_4
#define CAN_TXPIN           GPIO_NUM_5

#define CAN_TIMEOUT     5 // seconds

// UART GPS params //
#define GPS_RXPIN       16
#define GPS_TXPIN       17
#define GPS_BAUDRATE    9600

#define GPS_TIMEOUT     30 // seconds 

// NTP server
#define NTP_SERVER      "asia.pool.ntp.org"

enum ID {
    logger_id, machine_id, ip_address, lat, lon, 
    sensor_status, gps_status, datetime, pile_id,
    depth, torque, incl_x, incl_y,
    description,
    LAST
};

const char* ID_NAMES[] = {
    "logger_id", "machine_id", "ip_address", "lat", "lon",
    "sensor_status", "gps_status", "datetime", "pile_id",
    "depth", "torque", "incl_x", "incl_y",
    "description"
};

const char* DEFAULT_VALUES[] = {
    "test", "testMachine", "0.0.0.0", "0.0", "0.0",
    "0", "0", "0000-00-00_00-00-00", "",
    "0.0", "0.0", "0.0", "0.0",
    "SANY drilling rig sensor readings"
};

#define BLAST_FILENAME          "/blast"

// payload fields //
#define NUM_CRANE_PAYLOAD_FIELDS  4
const char* CRANE_PAYLOAD_FIELDS[] = {
    "depth", "torque", "x_inclination", "y_inclination"
};

const int CRANE_PAYLOAD_FIELD_IDS[] = {
    ID::depth, ID::torque, ID::incl_x, ID::incl_y
};

// CAN IDs //
#define CANID_INCLINE   "18050710"
#define CANID_DEPTH     "18050750"
#define CANID_TORQUE    "18050768"

#endif