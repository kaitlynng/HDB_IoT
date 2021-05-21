#ifndef SYSTEM_CONFIG_H_
#define SYSTEM_CONFIG_H_

#include <Arduino.h>

// RTC params //
#define SD_CS                15
#define GMTOFFSET_SEC        28800 //GMTOFFSET * 60 * 60 //https://en.wikipedia.org/wiki/List_of_UTC_time_offsets REFER TO LINK FOR OFFSET NUMBER
#define DAYLIGHTOFFSET_SEC   0
#define RTC_TIMEOUT          10 // seconds

// ESP sleep //
#define FORMAT_LITTLEFS_IF_FAILED   true
#define INTERRUPT_ATTR              IRAM_ATTR
#define MINSET                      1
#define uS_TO_S_FACTOR              1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP               60 * MINSET /* Time ESP32 will go to sleep (in seconds) */

// MQTT params //
#define MQTT_RECON      0

// UART GPS //
#define GPS_RXPIN       16
#define GPS_TXPIN       17
#define GPS_BAUDRATE    9600

// OLED //
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1

enum ID {
    sender_id, sensor_id, event_title, event_id, event_type,
    block_num, contract_num, ip_address, 
    severity, description, target_depth, 
    lat, longi, hole_dia, hole_num,
    sensor_status, datetime,
    depth, torque, incl_x, incl_y, max_depth,
    prev_dt, prev_hole_num,
    csv_filename,
    LAST
};

// filenames //
#define NUM_STORAGE_FILENAMES   10

const char STORAGE_FILENAMES[][20] = {
    "/Hole_diameter", "/Hole_Number", "/initialdate.txt", 
    "/MachineID.txt", "/MachineID.txt", "/depthMAX.txt",
    "/BlockNO.txt", "/ContractNo.txt", "/CSVFilename.txt"
};

const int STORAGE_FILENAME_IDS[] = {
    ID::hole_dia, ID::hole_num, ID::prev_dt, 
    ID::sender_id, ID::sensor_id, ID::max_depth, 
    ID::block_num, ID::contract_num, ID::csv_filename
};

#define HTML_FILENAME       "/index.htm"

#define INPUTCSV_FILENAME   "/currentCSV.txt"

// SQL fields //
#define NUM_SQL_FIELDS      17
const char SQL_FIELDS[][20] = {
    "SenderID", "SensorID",
    "Lat", "Longi",
    "HoleDiameter", "HoleNumber",
    "EventID", "Sensorstatus", "Time", "Severity",
    "Depth", "Torque", "InclinationX", "InclinationY",
    "Description", "Address2", "MaxDepth"
};

const int SQL_FIELD_IDS[] = {
    ID::sender_id, ID::sensor_id,
    ID::lat, ID::longi,
    ID::hole_dia, ID::hole_num,
    ID::event_title, ID::sensor_status,
    ID::datetime, ID::severity,
    ID::depth, ID::torque, ID::incl_x, ID::incl_y,
    ID::description, ID::ip_address, ID::max_depth
};

// csv fields //
#define NUM_CSV_FIELDS  14

const char CSV_FIELDS[][20] = {
    "Send ID", "Sensor ID", "Lattitude", "Longitude",
    "Event ID", "Event Type", "Sensor Status", 
    "Date and Time", "Severity", "Depth", "Torque", 
    "InclineX", "InclineX", "InclineY", "Description"
};

const int CSV_FIELD_IDS[] = {
    ID::sender_id, ID::sensor_id, ID::lat, ID::longi, 
    ID::event_id, ID::event_type, ID::sensor_status,
    ID::datetime, ID::severity, ID::depth, ID::torque,
    ID::incl_x, ID::incl_y, ID::description
};

// email fields //
#define NUM_EMAIL_FIELDS    14

const char EMAIL_FIELDS[][20] = {
    "EventID", "Event type", "Description", "Severity", 
    "\nSenderID", "SensorID", "Starting time", 
    "\nHole Nnmber", "Resource path", "", "Intended depth",
    "\nTime", "Sensor status", "Max depth"
};

const int EMAIL_FIELD_IDS[] = {
    ID::event_id, ID::event_type, ID::description, ID::severity,
    ID::sender_id, ID::sensor_id, ID::prev_dt,
    ID::prev_hole_num, ID::lat, ID::longi, ID::target_depth,
    ID::datetime, ID::sensor_status, ID::max_depth
};

// http req fields //
#define NUM_HTTP_FIELDS     6
const char HTTP_FIELDS[][20] = {
    "input1", "input2", "input3", 
    "input4", "input4", "input5", "input6", 
};

const int HTTP_FIELD_IDS[] = {
    ID::hole_dia, ID::hole_num, ID::target_depth, 
    ID::sender_id, ID::sensor_id, ID::block_num, ID::contract_num
}; // HTML web page to handle 2 input fields (input1, input2)

// json fields //
#define NUM_JSON_FIELDS     15
const char JSON_FIELDS[][20] = {
    "SenderID", "SensorID", "ResourcePath", "ResourcePath", 
    "Holediameter", "HoleNumber", "EventID", 
    "sensorstatus", "Time", "severity", 
    "Depth", "Torque", "InclinationX", "InclinationY", "Description"
};

const int JSON_FIELD_IDS[] = {
    ID::sender_id, ID::sensor_id, ID::lat, ID::longi, 
    ID::hole_dia, ID::hole_num, ID::event_type, 
    ID::sensor_status, ID::datetime, ID::severity, 
    ID::depth, ID::torque, ID::incl_x, ID::incl_y, ID::description
};

// CAN params //
#define CAN_RX_QUEUE_SIZE   151
#define CAN_RXPIN           GPIO_NUM_4
#define CAN_TXPIN           GPIO_NUM_5

#define CAN_TIMEOUT     10 // seconds

// CAN IDs //
#define CANID_INCLINE   "18050710"
#define CANID_DEPTH     "18050750"
#define CANID_TORQUE    "18050768"


#endif