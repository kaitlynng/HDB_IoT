#include "../config/system_config.h"
#include "../config/user_config.h"

#include <Arduino.h>
#include <time.h>  //time library
#include <Wire.h> //communicate with i2c devices
#include <WiFi.h> //wifi library from the ESP32 

#include <RTClib.h> //a fork of Jeelab's RTC library for Arduino

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> //Async HTTP and WebSocket Server for ESP8266 Arduino

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "modules/can_wrapper.h"
#include "modules/gps_wrapper.h"
#include "modules/sd_wrapper.h"

#include "modules/sql_wrapper.h"
#include "modules/email_wrapper.h"
#include "modules/mqtt_wrapper.h"

#include "utils/fmt_util.h"

WiFiClient client;                 // Use this for WiFi instead of EthernetClient
RTC_DS3231 rtc;

AsyncWebServer server(80);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// constants
const TimeSpan update_interval = TimeSpan(UPDATE_RATE);

// objects
CanWrapper can_wrapper(CAN_TXPIN, CAN_RXPIN, CAN_RX_QUEUE_SIZE, CAN_SPEED_250KBPS);
SqlWrapper sql_wrapper(MYSQL_SERVER_ADDRESS, MYSQL_USER, MYSQL_PASS, client);
GpsWrapper gps_wrapper(GPS_TXPIN, GPS_RXPIN, GPS_BAUDRATE);
SdWrapper sd_wrapper(SD_CS);
EmailWrapper email_wrapper(SMTP_SERVER, SMTP_SERVER_PORT);
MqttWrapper mqtt_wrapper(client, MQTT_SERVER, MQTT_SERVER_PORT, MQTT_CLIENT);

// variables
String can_data[3]; // {can_id, msb, lsb}
double gps_data[2]; // {lat, longi}
const int cbuff_size = 800;
char cbuff[cbuff_size];

char ip_cbuff[20];

char data_c[ID::LAST][50];
uint32_t data_i[ID::LAST];
float data_f[ID::LAST]; 

char filenames_arr[ID::LAST][20];

DateTime dt_now;
DateTime dt_next;

void store(int id, char* val) {
  strcpy(data_c[id], val);
  if (filenames_arr[id][0] != '\0') {
    sd_wrapper.write_file(filenames_arr[id], val);
  }
}

void store(int id, int val) {
  data_i[id] = val;
  sprintf(data_c[id], "%d", val);
}

void store(int id, float val) {
  data_f[id] = val;
  sprintf(data_c[id], "%f", val);
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

String processor(const String &var) { // TODO 
  if (var == "Holediameter") {
    return data_c[ID::hole_dia];

  } else if (var == "input1") {
      return data_c[ID::hole_dia];

  } else if (var == "input2") {
      store(ID::hole_num, var);
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

  } else {
      return "";
  }
}

void parse_can(String can_id, String msb, String lsb) {
  char* msb_c;
  char* lsb_c;

  msb.toCharArray(msb_c, 16);
  lsb.toCharArray(lsb_c, 16);

  if (can_id == CANID_INCLINE) {
    store(ID::incl_x, lsb);
    store(ID::incl_y, msb);
    
  } else if (can_id == CANID_DEPTH) {
    store(ID::depth, lsb.toFloat());
    if (data_f[ID::depth] > data_f[ID::max_depth]) {
      store(ID::max_depth, data_f[ID::depth]);
      sd_wrapper.write_file(filenames_arr[ID::max_depth], data_c[ID::max_depth]);
    }
    
  } else if (can_id == CANID_TORQUE) {
    store(ID::torque, float((lsb.toFloat() / 32.0) * 205.0));
  }
}

void WIFI_connect() {
  unsigned long startTime = millis();

  Serial.printf("Connecting to %s ", NETWORK_SSID);
  WiFi.begin(NETWORK_SSID, NETWORK_PASS);

  // try to connect every 0.1s for wifi connection timeout
  while (millis() - startTime < NETWORK_TIMEOUT) {
    if (WiFi.status() == WL_CONNECTED) break;
    delay(100);
    Serial.print(".");
  }

  switch (WiFi.status()) 
  {
    case 6 : Serial.print("Wifi not connected restarting esp"); ESP.restart(); break;
    case 3 : Serial.println("Wifi connected continue with ops"); break;
    case 1 : Serial.println("No wifi connected"); ESP.restart(); break; //append_file(SD, "/error_log", "No wifi \n");
    default : Serial.print("Unknown code: "); Serial.println(WiFi.status());
  }
}

void sync_rtc_time() {
  Serial.print("Connecting to RTC...");
  long stamp = millis();
  while (!rtc.begin() && millis() - stamp < RTC_TIMEOUT * 1000) {
    delay(10);
  }

  if (!rtc.begin()) {
    Serial.println("WARNING: Couldn't connect to RTC.");
    return;
  }
  
  Serial.println("OK. Adjusting time...");

  if (WiFi.status() != 3) {
    Serial.println("No WiFi connection! Unable to adjust time.");
    return;
  }

  configTime(GMTOFFSET_SEC, DAYLIGHTOFFSET_SEC, NTP_SERVER);
  
  struct tm t;
  if (!getLocalTime(&t)) {
    Serial.println("Failed to get time info from ntp server.");
    return;
  }

  rtc.adjust(tm2DateTime(t));
  Serial.println("Successfully synced RTC to NTP!");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //void MQTT_connect();
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  Serial.println("Configured all RTC Peripherals to be powered down in sleep");  

  //connect to WiFi
  WIFI_connect();
  ip2String(WiFi.localIP(), ip_cbuff);
  store(ID::ip_address, ip_cbuff); // probably need error handling

  //setup modules
  can_wrapper.setup();
  sql_wrapper.setup();
  gps_wrapper.setup();
  sd_wrapper.setup();
  email_wrapper.setup(); //doesn't do anything, for standardisation
  mqtt_wrapper.setup();

  Serial.println("Test1");

  for (int i = 0; i < NUM_STORAGE_FILENAMES; i++) {
    if (i < ID::LAST) strcpy(filenames_arr[STORAGE_FILENAME_IDS[i]], STORAGE_FILENAMES[i]);
  }
  Serial.println("Test2");

  // initialise data values from SD card
  for (int i = 0; i < NUM_STORAGE_FILENAMES; i++) {
    data_c[STORAGE_FILENAME_IDS[i]] = sd_wrapper.read_file(STORAGE_FILENAMES[i]);
  }
  Serial.println("Test3");

  //for max_depth, need the float value
  store(ID::max_depth, data_c[ID::max_depth].toFloat());

  Serial.println("Test4");

  //Setup time synchronisation
  sync_rtc_time();

  Serial.println("Test5");
  
  Serial.println("Mirror Demo - ESP32-Arduino-CAN");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, HTML_FILENAME, "text/html", processor);
  });

  server.on("/sendemail", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, HTML_FILENAME, "text/html", processor);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    for (int i = 0; i < NUM_HTTP_FIELDS; i++) {
      if (request->hasParam(HTTP_FIELDS[i])) {
        store(HTTP_FIELD_IDS[i], request->getParam(HTTP_FIELDS[i])->value());
        break;
      }
    }
    request->send(SD, HTML_FILENAME, "text/html", processor);
  });

  // Start server
  server.begin();

  dt_now = rtc.now();
  dt_next = rtc.now();


}

void loop() {
  // put your main code here, to run repeatedly:
  sync_rtc_time();
  dt_now = rtc.now();
  store(ID::datetime, dt_now.toString((char*)"YYYY-MM-DD_hh-mm-ss")); //check cast!

  //update all data values
  if (can_wrapper.poll(can_data)) {
    parse_can(can_data[0], can_data[1], can_data[2]);
  }
  
  if (gps_wrapper.poll(gps_data)) {
    store(ID::lat, (float)gps_data[0]);
    store(ID::longi, (float)gps_data[1]);
  }

  if (dt_now > dt_next) {
    // Send data
    String sql_msg = format_sql_msg(DEFAULT_DB, DEFAULT_TABLE, NUM_SQL_FIELDS, SQL_FIELDS, SQL_FIELD_IDS, data_c);
    sql_msg.toCharArray(cbuff, cbuff_size);
    sql_wrapper.insert(cbuff);
    memset(cbuff, 0, cbuff_size);

    String csv_msg = format_csv_msg(NUM_CSV_FIELDS, CSV_FIELDS, CSV_FIELD_IDS, data_c);
    sd_wrapper.append_file(data_c[ID::csv_filename], csv_msg);

    String json_msg = format_json_msg(NUM_JSON_FIELDS, JSON_FIELDS, JSON_FIELD_IDS, data_c);
    json_msg.toCharArray(cbuff, cbuff_size);
    mqtt_wrapper.publish(MQTT_TOPIC, cbuff);
    memset(cbuff, 0, cbuff_size);

    dt_next = dt_now + update_interval;
  }

  if (data_c[ID::prev_hole_num] != data_c[ID::hole_num]) {
    //hole changed, send mail
    String email_subject = CONTRACT_NAME + '/' + 
                          data_c[ID::contract_num] + "/" +
                          data_c[ID::hole_num];

    String email_msg = format_email_msg(NUM_EMAIL_FIELDS, EMAIL_FIELDS, EMAIL_FIELD_IDS, data_c);
    
    email_wrapper.send(EMAIL_SENDER_NAME, EMAIL_SENDER_ACCOUNT, EMAIL_SENDER_PASS, 
                      EMAIL_NUM_RECIPIENTS, EMAIL_RECIPIENTS,
                      email_subject, email_msg, data_c[ID::csv_filename]);

    //update previous values to current value
    store(ID::prev_hole_num, data_c[ID::hole_num]);
    store(ID::prev_dt, data_c[ID::datetime]);

    //reset depth value
    store(ID::max_depth, float(0.0));

    //update csv filename
    String new_csv_filename = "/" + data_c[ID::event_title] + "_" + data_c[ID::hole_num] + "_" + data_c[ID::datetime] + ".csv";
    store(ID::csv_filename, new_csv_filename);

    //create new csv file
    String csv_header = format_csv_header(NUM_CSV_FIELDS, CSV_FIELDS);
    sd_wrapper.write_file(data_c[ID::csv_filename], csv_header);

  } //interrupt?

}