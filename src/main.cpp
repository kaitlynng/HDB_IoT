#include <Arduino.h>

#include <string>
#include <time.h>  //time library
#include <Wire.h> //communicate with i2c devices
#include <WiFi.h>

#include <RTClib.h>            //a fork of Jeelab's RTC library for Arduino

#include <HTTPClient.h>


#include "../config/system_config.h"

#if __has_include("../config/user_config.h")
#  include "../config/user_config.h"
#else
#  include "../config/default_user_config.h"
#endif

#include "modules/wifi_wrapper.h"
#include "modules/can_wrapper.h"
#include "modules/gps_wrapper.h"

#include "utils/fmt_util.h"

WiFiClient client;

RTC_DS3231 rtc;

// constants
const TimeSpan update_interval = TimeSpan(UPDATE_RATE);
const TimeSpan ntp_interval = TimeSpan(RTC_SYNC_RATE);
const TimeSpan gps_interval = TimeSpan(GPS_TIMEOUT);
const TimeSpan can_interval = TimeSpan(CAN_TIMEOUT); 

const char datetime_fmt[20] = "YYYY-MM-DD hh:mm:ss";

// objects
WiFiWrapper wifi_wrapper(NETWORK_SSID, NETWORK_PASS, NETWORK_TIMEOUT);
CanWrapper can_wrapper(CAN_RXPIN, CAN_TXPIN, CAN_RX_QUEUE_SIZE, CAN_SPEED_250KBPS);
GpsWrapper gps_wrapper(GPS_RXPIN, GPS_TXPIN, GPS_BAUDRATE);

// variables
String can_data[3]; // {can_id, msb, lsb}
char msb_c[50];
char lsb_c[50];
double gps_data[2]; // {lat, longi}
char dt_buf[50];

const int cbuff_size = 800;
char cbuff[cbuff_size] = "";

const int data_size = 50;
char data_c[ID::LAST][data_size];
uint32_t data_i[ID::LAST];
float data_f[ID::LAST]; 

// HTTP stuff
char endpoint[100];
String response;
StaticJsonDocument<800> response_doc;

// log filenames (ignore logging for now)
// char data_log_fname[50];
// char error_log_fname[50];

DateTime dt_now;
DateTime dt_next;

DateTime last_can_update;
DateTime last_gps_update;

DateTime next_ntp_update;

int is_sensor_online = 0;
int is_gps_online = 0;
int blast_mode = 0;

int executing_job = 0;

void parse_can(String can_id, String msb, String lsb) {
  msb.toCharArray(msb_c, 16);
  lsb.toCharArray(lsb_c, 16);

  if (can_id == CANID_INCLINE) {
    data_f[ID::incl_x] = lsb.toFloat();
    data_f[ID::incl_y] = msb.toFloat();
    strncpy(data_c[ID::incl_x], lsb_c, data_size);
    strncpy(data_c[ID::incl_y], msb_c, data_size);

    is_sensor_online = 1;
     
  } else if (can_id == CANID_DEPTH) {
    data_f[ID::depth] = lsb.toFloat();
    strncpy(data_c[ID::depth], lsb_c, data_size);

    is_sensor_online = 1;
    
  } else if (can_id == CANID_TORQUE) {
    data_f[ID::torque] = (lsb.toFloat() / 32.0) * 205.0;
    snprintf(data_c[ID::torque], data_size, "%f", data_f[ID::torque]);

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
  Serial.begin(115200);

  // initialise with default values
  for (int i = 0; i < ID::LAST; i++) {
    snprintf(data_c[i], data_size, "%s", DEFAULT_VALUES[i]);
  }

  // set Logger ID
  strncpy(data_c[ID::logger_id], LOGGER_ID, data_size);

  // set Machine ID
  strncpy(data_c[ID::machine_id], MACHINE_ID, data_size);

  // set latitude
  data_f[ID::lat] = atof(DEFAULT_VALUES[ID::lat]);
  data_f[ID::lon] = atof(DEFAULT_VALUES[ID::lon]);

  // set longitude

  wifi_wrapper.set_static_ip_params(LOCAL_IP_ADDRESS, GATEWAY_ADDRESS, SUBNET_ADDRESS, PRIMARY_DNS, SECONDARY_DNS);
  if (!wifi_wrapper.setup()) {
    // restart ESP if wifi is not available!
    ESP.restart();
  };

  char ip_cbuff[20];
  wifi_wrapper.get_local_ip(ip_cbuff);
  strncpy(data_c[ID::ip_address], ip_cbuff, data_size);


  //Setup time synchronisation
  Serial.print("Connecting to RTC....");
  if (!connect_rtc()) {
    Serial.println("FATAL ERROR: UNABLE TO CONNECT TO RTC. Restarting ESP32...");
    ESP.restart();
  }
  Serial.println("OK.");

  sync_rtc_time();

  //setup modules
  can_wrapper.setup();
  gps_wrapper.setup();

  // set datetime
  strncpy(dt_buf, datetime_fmt, 50);
  strncpy(data_c[ID::datetime], rtc.now().toString(dt_buf), data_size);

  // set blast_mode first
  blast_mode = 1;

  dt_now = rtc.now();
  dt_next = rtc.now();

  next_ntp_update = rtc.now();
  last_can_update = rtc.now();
  last_gps_update = rtc.now();

  // poll gps once
  if (gps_wrapper.poll(gps_data)) {
    is_gps_online = 1;
    data_f[ID::lat] = (float)gps_data[0];
    data_f[ID::lon] = (float)gps_data[1];

    snprintf(data_c[ID::lat], data_size, "%f", gps_data[0]);
    snprintf(data_c[ID::lon], data_size, "%f", gps_data[1]);
  }

  // send power up
  HTTPClient http;
  snprintf(endpoint, 100, "%s/%s", HTTP_ENDPOINT, "on");
  Serial.println(endpoint);
  http.begin(client, endpoint);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");

  format_heartbeat_msg(data_f[ID::lat], data_f[ID::lon], cbuff_size, cbuff);
  Serial.println(cbuff);
  int httpResponseCode = http.POST(cbuff);

  if (httpResponseCode > 0) {
    Serial.println("Power on successful!");
    response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Unable to power on! HTTP Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

}

void loop() {
  // ArduinoOTA.handle();

  if ((wifi_wrapper.get_wifi_status() == WL_CONNECTED) && blast_mode) {
    // remove blast first!
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
  strncpy(data_c[ID::datetime], dt_now.toString(dt_buf), data_size);

  // update all data values
  if (can_wrapper.poll(can_data)) {
    parse_can(can_data[0], can_data[1], can_data[2]);
  }
  
  if (is_sensor_online) {
    strncpy(data_c[ID::sensor_status], "1", data_size);
    last_can_update = dt_now;
  } else if (dt_now > last_can_update + can_interval) {
    strncpy(data_c[ID::sensor_status], "0", data_size);
  }

  if (gps_wrapper.poll(gps_data)) {
    is_gps_online = 1;
    data_f[ID::lat] = (float)gps_data[0];
    data_f[ID::lon] = (float)gps_data[1];
    snprintf(data_c[ID::lat], data_size, "%f", gps_data[0]);
    snprintf(data_c[ID::lon], data_size, "%f", gps_data[1]);
  }

  if (is_gps_online) {
    strncpy(data_c[ID::gps_status], "1", data_size);
    last_gps_update = dt_now;
  } else if (dt_now > last_gps_update + gps_interval) {
    strncpy(data_c[ID::gps_status], "0", data_size);
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
      blast_mode = 1;
      
    } else {
      HTTPClient http;
      snprintf(endpoint, 100, "%s/%s", HTTP_ENDPOINT, "hb");
      Serial.print("Endpoint: ");
      Serial.println(endpoint);

      http.begin(client, endpoint);

      http.addHeader("Content-Type", "application/json");
      http.addHeader("Accept", "application/json");
      
      if (executing_job) {
        // send job
        format_job_msg(data_f[ID::lat], data_f[ID::lon], data_i[ID::pile_id], data_c[ID::description],
                       data_c[ID::datetime], data_f[ID::lat], data_f[ID::lon], 
                       data_c[ID::sensor_status], data_c[ID::gps_status],
                       NUM_CRANE_PAYLOAD_FIELDS, CRANE_PAYLOAD_FIELDS, CRANE_PAYLOAD_FIELD_IDS, data_f,
                       cbuff_size, cbuff);

      } else {
        // send heartbeat
        format_heartbeat_msg(data_f[ID::lat], data_f[ID::lon], cbuff_size, cbuff);
      }

      Serial.println(cbuff);

      int httpResponseCode = http.POST(cbuff);

      if (httpResponseCode > 0) {
        response = http.getString();
        Serial.print("Response: ");
        Serial.println(response);

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(response_doc, response);

        // Test if parsing succeeds.
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
        } else {
          if (response_doc.containsKey("job")) {
            executing_job = 1;
            data_i[ID::pile_id] = response_doc["job"]["id"];
            snprintf(data_c[ID::pile_id], data_size, "%d", data_i[ID::pile_id]);
            
          } else {
            executing_job = 0;
          }
        }

      } else {
        Serial.print("HTTP Error code: ");
        Serial.println(httpResponseCode);

        response = http.getString();
        Serial.println(response);
        
      }

      http.end();
    }

    dt_next = dt_now + update_interval; 
  }

  is_sensor_online = 0;
  is_gps_online = 0;
}

