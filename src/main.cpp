#include <Arduino.h>
#include <string>
#include <time.h>  //time library
#include <Wire.h> //communicate with i2c devices
#include <WiFiClientSecure.h>

#include <RTClib.h> //a fork of Jeelab's RTC library for Arduino

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> //Async HTTP and WebSocket Server for ESP8266 Arduino

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "../config/system_config.h"

#if __has_include("../config/user_config.h")
#  include "../config/user_config.h"
#else
#  include "../config/default_user_config.h"
#endif

#include "loggers/serial_logger.h"

#include "modules/wifi_wrapper.h"
#include "modules/can_wrapper.h"
#include "modules/gps_wrapper.h"
#include "modules/sd_wrapper.h"

#include "modules/sql_wrapper.h"
#include "modules/email_wrapper.h"
#include "modules/mqtt_wrapper.h"

#include "utils/fmt_util.h"


WiFiClient client; // should move WiFiSecureClient out from mqtt wrapper too

RTC_DS3231 rtc;

AsyncWebServer server(80);

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// constants
const TimeSpan update_interval = TimeSpan(UPDATE_RATE);
const TimeSpan ntp_interval = TimeSpan(RTC_SYNC_RATE);
const TimeSpan gps_interval = TimeSpan(GPS_TIMEOUT);
const TimeSpan can_interval = TimeSpan(CAN_TIMEOUT); 

const char datetime_fmt[20] = "YYYY-MM-DD_hh-mm-ss";
char status_online[7] = "online";
char status_offline[8] = "offline";

char aws_cert_ca[2000];
char aws_cert_crt[2000];
char aws_cert_private[2000];

// objects
SerialLogger serial_logger(LogLevel::LOG_DEBUG);
WiFiWrapper wifi_wrapper(NETWORK_SSID, NETWORK_PASS, NETWORK_TIMEOUT);

CanWrapper can_wrapper(CAN_RXPIN, CAN_TXPIN, CAN_RX_QUEUE_SIZE, CAN_SPEED_250KBPS);
GpsWrapper gps_wrapper(GPS_RXPIN, GPS_TXPIN, GPS_BAUDRATE);
SdWrapper sd_wrapper(SD_CS);

SqlWrapper sql_wrapper(MYSQL_HOSTNAME, MYSQL_PORT, MYSQL_USER, MYSQL_PASS, client, SQL_FLAG);
MqttWrapper mqtt_wrapper(MQTT_ENDPOINT, MQTT_PORT, MQTT_CLIENT, MQTT_TIMEOUT, MQTT_FLAG);
EmailWrapper email_wrapper(SMTP_SERVER, SMTP_SERVER_PORT, EMAIL_FLAG);

// variables
String can_data[3]; // {can_id, msb, lsb}
double gps_data[2]; // {lat, longi}
char dt_buf[50];

const int cbuff_size = 800;
char cbuff[cbuff_size] = "";

char data_c[ID::LAST][50];
uint32_t data_i[ID::LAST];
float data_f[ID::LAST]; 
bool data_updated[ID::LAST];

char filenames_arr[ID::LAST][FILENAME_SIZE];
char csv_path[FILENAME_SIZE];

DateTime dt_now;
DateTime dt_next;

DateTime last_can_update;
DateTime last_gps_update;

DateTime next_ntp_update;

int is_sensor_online = 0;
int is_gps_online = 0;
int blast_mode = 0;

void store(int id, char* val) {
  strncpy(data_c[id], val, 50);
  if (filenames_arr[id][0] != '\0') {
    if (strcmp(val, DEFAULT_VALUES[id]) != 0) { // check if it's a default value
      sd_wrapper.write_file(filenames_arr[id], val);
    }
  }
}

void store(int id, int val) {
  data_i[id] = val;
  char buf[100];
  snprintf(buf, 100, "%d", val);
  store(id, buf);
}

void store(int id, float val) {
  data_f[id] = val;
  char buf[100];
  snprintf(buf, 100, "%f", val);
  store(id, buf);
}

void reset_flags() {
  for (int i = 0; i < ID::LAST; i++) {
    data_updated[i] = false;
  }
}

String processor(const String &var) { // TODO 
  if (var == "Holediameter") {
    return data_c[ID::hole_dia];

  } else if (var == "input1") {
      return data_c[ID::hole_dia];

  } else if (var == "input2") {
      char var_c[50];
      var.toCharArray(var_c, 50);
      store(ID::hole_num, var_c);
      return data_c[ID::hole_num];

  } else if (var == "Holenumber") {
      return data_c[ID::hole_num];

  } else if (var == "PileLength") {
      return data_c[ID::target_depth];

  } else if (var == "MachineID") {
      return data_c[ID::sender_id];

  } else if (var == "BlockNO") {
      return data_c[ID::block_num];

  } else if (var == "ContractNo") {
      return data_c[ID::contract_num];
  
  } else if (var == "LoggerID") {
    return data_c[ID::logger_id];

  } else {
      return "";
  }
}

void parse_can(String can_id, String msb, String lsb) {
  char msb_c[50];
  char lsb_c[50];

  msb.toCharArray(msb_c, 16);
  lsb.toCharArray(lsb_c, 16);

  if (can_id == CANID_INCLINE) {
    if (!data_updated[ID::incl_x]) {
      store(ID::incl_x, lsb_c);
      data_updated[ID::incl_x] = true;
    }
    if (!data_updated[ID::incl_y]) {
      store(ID::incl_y, msb_c);
      data_updated[ID::incl_y] = true;
    }
    is_sensor_online = 1;
     
  } else if (can_id == CANID_DEPTH) {
    if (!data_updated[ID::depth]) {       // only take the lastest depth value...
      store(ID::depth, lsb.toFloat());
      data_updated[ID::depth] = true;
    }

    if (lsb.toFloat() > data_f[ID::max_depth]) {   // but pick the max depth value of everything
      store(ID::max_depth, data_f[ID::depth]);
    }
    is_sensor_online = 1;
    
  } else if (can_id == CANID_TORQUE) {
    if (!data_updated[ID::torque]) {
      store(ID::torque, float((lsb.toFloat() / 32.0) * 205.0));
      data_updated[ID::torque] = true;
    }
    is_sensor_online = 1;
  }
}


bool connect_rtc() {
  long stamp = millis();
  while (!rtc.begin() && millis() - stamp < RTC_TIMEOUT * 1000) {
    delay(10);
  }

  if (!rtc.begin()) {
    Serial.println("WARNING: Couldn't connect to RTC.");
    return false;
  }

  return true;
}

void sync_rtc_time() {
  Serial.print("Adjusting time...");

  if (wifi_wrapper.get_wifi_status() != 3) {
    Serial.println("No WiFi connection! Unable to adjust time.");
    return;
  }

  configTime(GMTOFFSET_SEC, DAYLIGHTOFFSET_SEC, NTP_SERVER);
  
  struct tm t;
  if (!getLocalTime(&t)) {
    Serial.println("Failed to get time info from ntp server. RTC is not synced!");
    return;
  }

  rtc.adjust(tm2DateTime(t));
  Serial.println("Successfully synced RTC to NTP!");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  serial_logger.log_debug("Test debug!");
  serial_logger.log_info("Test loginfo!");
  serial_logger.log_warn("Test logwarn!");
  serial_logger.log_error("Test log error!");
  serial_logger.log_fatal("Test fatal!");

  // initialise with default values
  for (int i = 0; i < ID::LAST; i++) {
    char* default_value = strdup(DEFAULT_VALUES[i]);
    store(i, default_value);
    free(default_value);
    data_updated[i] = false;
  }

  // set Logger ID
  strncpy(data_c[ID::logger_id], LOGGER_ID, 50);

  wifi_wrapper.set_static_ip_params(LOCAL_IP_ADDRESS, GATEWAY_ADDRESS, SUBNET_ADDRESS, PRIMARY_DNS, SECONDARY_DNS);
  wifi_wrapper.setup();
  
  char ip_cbuff[20];
  wifi_wrapper.get_local_ip(ip_cbuff);
  store(ID::ip_address, ip_cbuff); // probably need error handling

  // setup Arduino OTA
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  //Setup time synchronisation
  Serial.print("Connecting to RTC....");
  if (!connect_rtc()) {
    Serial.println("FATAL ERROR: UNABLE TO CONNECT TO RTC. Restarting ESP32...");
    // ESP.restart();
  }
  Serial.println("OK.");

  sync_rtc_time();

  //setup modules
  can_wrapper.setup();
  sql_wrapper.setup();

  gps_wrapper.setup();
  sd_wrapper.setup();
  email_wrapper.setup(); //doesn't do anything, for standardisation

  // read mqtt certificates
  sd_wrapper.read_file(AWS_CERT_CA_FILENAME, 2000, aws_cert_ca);
  sd_wrapper.read_file(AWS_CERT_CRT_FILENAME, 2000, aws_cert_crt);
  sd_wrapper.read_file(AWS_CERT_PRIVATE_FILENAME, 2000, aws_cert_private);
  mqtt_wrapper.set_certificates(aws_cert_ca, aws_cert_crt, aws_cert_private);
  mqtt_wrapper.setup();

  for (int i = 0; i < NUM_STORAGE_FILENAMES; i++) {
    if (i < ID::LAST) strncpy(filenames_arr[STORAGE_FILENAME_IDS[i]], STORAGE_FILENAMES[i], FILENAME_SIZE);
  }

  // initialise data values from SD card
  for (int i = 0; i < NUM_STORAGE_FILENAMES; i++) {
    char* filename_c = strdup(STORAGE_FILENAMES[i]);
    sd_wrapper.read_file(filename_c, 50, data_c[STORAGE_FILENAME_IDS[i]]);
    free(filename_c);
  }

  // for initial datetime, change to current datetime if default
  if (strcmp(data_c[ID::prev_dt], DEFAULT_VALUES[ID::prev_dt]) == 0) {
    Serial.println("Setting default datetime to current datetime...");
    strncpy(dt_buf, datetime_fmt, 50);
    store(ID::prev_dt, rtc.now().toString(dt_buf));
  }

  // set datetime
  strncpy(dt_buf, datetime_fmt, 50);
  store(ID::datetime, rtc.now().toString(dt_buf));

  // for initial csv, change csv filename if default
  if (strcmp(data_c[ID::csv_filename], DEFAULT_VALUES[ID::csv_filename]) == 0) {
    // create new csv filename
    Serial.println("No existing csv file found! Creating new csv file...");
    char new_csv_filename[100];
    snprintf(new_csv_filename, 100, "%s_%s_%s.csv", data_c[ID::event_title], data_c[ID::prev_hole_num], data_c[ID::prev_dt]);
    if (DEBUG_FLAG) {Serial.println(new_csv_filename); }
    store(ID::csv_filename, new_csv_filename);

    //create new csv file
    format_csv_header(NUM_CSV_FIELDS, CSV_FIELDS, cbuff_size, cbuff);
    if (DEBUG_FLAG) {Serial.println(cbuff); }
    snprintf(csv_path, FILENAME_SIZE, "/%s", data_c[ID::csv_filename]);
    sd_wrapper.write_file(csv_path, cbuff);
  }

  //for max_depth, need the float value
  Serial.println("LOOK HERE");
  Serial.println(data_c[ID::max_depth]);
  Serial.println((float)atof(data_c[ID::max_depth]));
  store(ID::max_depth, (float)atof(data_c[ID::max_depth]));

  reset_flags();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, HTML_FILENAME, "text/html", false, processor);
  });

  server.on("/sendemail", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, HTML_FILENAME, "text/html", false, processor);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    char value_c[50];
    for (int i = 0; i < NUM_HTTP_FIELDS; i++) {
      if (request->hasParam(HTTP_FIELDS[i])) {
        request->getParam(HTTP_FIELDS[i])->value().toCharArray(value_c, 50);
        store(HTTP_FIELD_IDS[i], value_c);
        break;
      }
    }
    store(ID::sensor_id, data_c[ID::sender_id]); // HACK PLEASE FIX
    request->send(SD, HTML_FILENAME, "text/html", false, processor);
  });

  // Start server
  server.begin();

  Serial.println("Mirror Demo - ESP32-Arduino-CAN");

  // set blast_mode first
  blast_mode = 1;

  // send first email by default
  char blast_cbuff[50 * ID::LAST];
  format_blast_msg(ID::LAST, data_c, sizeof(blast_cbuff), blast_cbuff);
  sd_wrapper.append_file(BLAST_EMAIL_FILENAME, blast_cbuff);

  dt_now = rtc.now();
  dt_next = rtc.now();

  next_ntp_update = rtc.now();
  last_can_update = rtc.now();
  last_gps_update = rtc.now();

  store(ID::prev_hole_num, data_c[ID::hole_num]);
}

void loop() {
  ArduinoOTA.handle();

  if ((wifi_wrapper.get_wifi_status() == WL_CONNECTED) && blast_mode) {
    // do a blast
    Serial.println("WiFi connection restored! Blasting messages...");

    // send sql and mqtt blast messages
    char ch;
    char blast_data_c[ID::LAST][50];
    int id = 0;
    unsigned int ptr = 0;

    if (sd_wrapper.is_file_available(BLAST_FILENAME)) {
      File blast_file = sd_wrapper.get_file(BLAST_FILENAME);
      while (blast_file.available()) {
        ch = blast_file.read();
        if (ch == ',') { // new field
          blast_data_c[id][ptr] = '\0'; //terminate string
          ptr = 0;
          id++;
        } else if (ch == '\n') { // new message
          format_sql_msg(DEFAULT_DB, DEFAULT_TABLE, NUM_SQL_FIELDS, SQL_FIELDS, SQL_FIELD_IDS, blast_data_c, cbuff_size, cbuff);
          if (DEBUG_FLAG) {Serial.println(cbuff); }
          sql_wrapper.insert(cbuff);

          format_json_msg(NUM_JSON_FIELDS, JSON_FIELDS, JSON_FIELD_IDS, blast_data_c, cbuff_size, cbuff);
          if (DEBUG_FLAG) {Serial.println(cbuff); }
          mqtt_wrapper.publish(MQTT_TOPIC, cbuff);

          ptr = 0;
          id = 0;
        } else {
          blast_data_c[id][ptr] = ch;
          ptr++;
        }
      }
      
      blast_file.close();

      sd_wrapper.remove_file(BLAST_FILENAME);

    }

    id = 0;
    ptr = 0;
    if (sd_wrapper.is_file_available(BLAST_EMAIL_FILENAME)) {
      File blast_file = sd_wrapper.get_file(BLAST_EMAIL_FILENAME);

      while (blast_file.available()) {
        ch = blast_file.read();
        if (ch == ',') { // new field
          blast_data_c[id][ptr] = '\0'; //terminate string
          ptr = 0;
          id++;
        } else if (ch == '\n') { // new message

          char email_subject[100];
          format_email_subject(blast_data_c, 100, email_subject);
          format_email_msg(NUM_EMAIL_FIELDS, EMAIL_FIELDS, EMAIL_FIELD_IDS, blast_data_c, cbuff_size, cbuff);
          if (DEBUG_FLAG) {Serial.println(email_subject); }
          if (DEBUG_FLAG) {Serial.println(cbuff); }

          bool attach_file;
          snprintf(csv_path, FILENAME_SIZE, "/%s", blast_data_c[ID::csv_filename]);
          if (sd_wrapper.is_file_available(csv_path)) {
            attach_file = true;
          } else {
            attach_file = false;
            Serial.println("WARNING: Unable to open csv file for email attachment!");
          }

          email_wrapper.send(EMAIL_SENDER_NAME, EMAIL_SENDER_ACCOUNT, EMAIL_SENDER_PASS, 
                          EMAIL_NUM_RECIPIENTS, EMAIL_RECIPIENTS,
                          email_subject, cbuff, blast_data_c[ID::csv_filename], csv_path, attach_file);

          ptr = 0;
          id = 0;

        } else {
          blast_data_c[id][ptr] = ch;
          ptr++;
        }
      }

      blast_file.close();

      sd_wrapper.remove_file(BLAST_EMAIL_FILENAME);

    }
    
    blast_mode = 0;

  }

  if (!connect_rtc()) {
    Serial.println("FATAL ERROR: UNABLE TO CONNECT TO RTC!");
    ESP.restart();
  }

  if (rtc.now() > next_ntp_update) {
    Serial.println("Sync rtc time!");
    sync_rtc_time();
    next_ntp_update = rtc.now() + ntp_interval;
  }

  dt_now = rtc.now();

  strncpy(dt_buf, datetime_fmt, 50);
  store(ID::datetime, dt_now.toString(dt_buf)); //check cast!

  //update all data values
  for (int i = 0; i < can_wrapper.get_num_frames_in_queue(); i++) {
    if (can_wrapper.poll(can_data)) {
      parse_can(can_data[0], can_data[1], can_data[2]);
    }
  }

  if (is_sensor_online) {
    last_can_update = dt_now;
    store(ID::sensor_status, status_online);
  } else if (dt_now > last_can_update + can_interval) {
    store(ID::sensor_status, status_offline);
  }

  if (gps_wrapper.poll(gps_data)) {
    is_gps_online = 1;
    store(ID::lat, (float)gps_data[0]);
    store(ID::longi, (float)gps_data[1]);
  }

  if (is_gps_online) {
    last_gps_update = dt_now;
    store(ID::gps_status, status_online);
  } else if (dt_now > last_gps_update + gps_interval) {
    store(ID::gps_status, status_offline);
  }

  if (dt_now > dt_next) {
    if (DEBUG_FLAG) {
      Serial.println("Data: ");
      for (int i = 0; i < ID::LAST; i++) {
        Serial.print(ID_NAMES[i]);
        Serial.print(": ");
        Serial.println(data_c[i]);
      }
      Serial.println("....................................");
    }

    if (wifi_wrapper.get_wifi_status() != WL_CONNECTED) {
      if (blast_mode == 0) {
        Serial.println("WARNING: WiFi connection dropped, switching to blast mode!");
      }
      
      wifi_wrapper.connect_unblocking();

      char blast_cbuff[50 * ID::LAST];
      format_blast_msg(ID::LAST, data_c, sizeof(blast_cbuff), blast_cbuff);
      sd_wrapper.append_file(BLAST_FILENAME, blast_cbuff);

      format_csv_msg(NUM_CSV_FIELDS, CSV_FIELDS, CSV_FIELD_IDS, data_c, cbuff_size, cbuff);
      snprintf(csv_path, FILENAME_SIZE, "/%s", data_c[ID::csv_filename]);
      sd_wrapper.append_file(csv_path, cbuff);

      blast_mode = 1;
      
    } else {

      // send normal messages (WiFi connected)
      format_sql_msg(DEFAULT_DB, DEFAULT_TABLE, NUM_SQL_FIELDS, SQL_FIELDS, SQL_FIELD_IDS, data_c, cbuff_size, cbuff);
      if (DEBUG_FLAG) {Serial.println(cbuff); }
      sql_wrapper.insert(cbuff);

      format_csv_msg(NUM_CSV_FIELDS, CSV_FIELDS, CSV_FIELD_IDS, data_c, cbuff_size, cbuff);
      if (DEBUG_FLAG) {Serial.println(cbuff); }
      snprintf(csv_path, FILENAME_SIZE, "/%s", data_c[ID::csv_filename]);
      sd_wrapper.append_file(csv_path, cbuff);
      
      format_json_msg(NUM_JSON_FIELDS, JSON_FIELDS, JSON_FIELD_IDS, data_c, cbuff_size, cbuff);
      if (DEBUG_FLAG) {Serial.println(cbuff); }
      mqtt_wrapper.publish(MQTT_TOPIC, cbuff);
    }

    dt_next = dt_now + update_interval; 
  }

  if (strcmp(data_c[ID::prev_hole_num], data_c[ID::hole_num]) != 0) {
    if (wifi_wrapper.get_wifi_status() != WL_CONNECTED) {
      wifi_wrapper.connect_unblocking();

      char blast_cbuff[50 * ID::LAST];
      format_blast_msg(ID::LAST, data_c, sizeof(blast_cbuff), blast_cbuff);
      sd_wrapper.append_file(BLAST_EMAIL_FILENAME, blast_cbuff);

      blast_mode = 1;

    } else {
      char email_subject[100];
      format_email_subject(data_c, 100, email_subject);
      format_email_msg(NUM_EMAIL_FIELDS, EMAIL_FIELDS, EMAIL_FIELD_IDS, data_c, cbuff_size, cbuff);
      if (DEBUG_FLAG) {Serial.println(email_subject); }
      if (DEBUG_FLAG) {Serial.println(cbuff); }

      bool attach_file;
      snprintf(csv_path, FILENAME_SIZE, "/%s", data_c[ID::csv_filename]);
      if (sd_wrapper.is_file_available(csv_path)) {
        attach_file = true;
      } else {
        attach_file = false;
        Serial.println("WARNING: Unable to open csv file for email attachment!");
      }

      email_wrapper.send(EMAIL_SENDER_NAME, EMAIL_SENDER_ACCOUNT, EMAIL_SENDER_PASS, 
                      EMAIL_NUM_RECIPIENTS, EMAIL_RECIPIENTS,
                      email_subject, cbuff, data_c[ID::csv_filename], csv_path, attach_file);
    }
    
    //update previous values to current value
    store(ID::prev_hole_num, data_c[ID::hole_num]);
    store(ID::prev_dt, data_c[ID::datetime]);

    //reset depth value
    store(ID::max_depth, float(0.0));

    //update csv filename
    char new_csv_filename[100];
    snprintf(new_csv_filename, 100, "%s_%s_%s.csv", data_c[ID::event_title], data_c[ID::hole_num], data_c[ID::prev_dt]);
    if (DEBUG_FLAG) {Serial.println(new_csv_filename); }
    store(ID::csv_filename, new_csv_filename);

    //create new csv file
    format_csv_header(NUM_CSV_FIELDS, CSV_FIELDS, cbuff_size, cbuff);
    if (DEBUG_FLAG) {Serial.println(cbuff); }
    snprintf(csv_path, FILENAME_SIZE, "/%s", data_c[ID::csv_filename]);
    sd_wrapper.write_file(csv_path, cbuff);
    
  }

  reset_flags();
  is_sensor_online = 0;
  is_gps_online = 0;
}