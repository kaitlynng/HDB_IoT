#include <Arduino.h>
#include <string>
#include <time.h>  //time library
#include <Wire.h> //communicate with i2c devices
#include <WiFiClientSecure.h>

// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>

#include "../config/system_config.h"

#if __has_include("../config/user_config.h")
#  include "../config/user_config.h"
#else
#  include "../config/default_user_config.h"
#endif

#include "modules/can_wrapper.h"


#include "utils/fmt_util.h"

uint32_t previousMillis = 0;
CanWrapper can_wrapper(CAN_RXPIN, CAN_TXPIN, CAN_RX_QUEUE_SIZE, CAN_SPEED_250KBPS);

// variables
String can_data[3]; // {can_id, msb, lsb}
double gps_data[2]; // {lat, longi}
char dt_buf[50];



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
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  Serial.println("Configured all RTC Peripherals to be powered down in sleep");  


 
  //setup modules
  can_wrapper.setup();
}

void loop() {
  uint32_t currentMillis = millis();
  int count = 0;
  if (currentMillis - previousMillis > 200){
    can_wrapper.send(CAN_ID[0], CAN_test_data_incline[count]);
    can_wrapper.send(CAN_ID[1], CAN_test_data_depth[count]);
    can_wrapper.send(CAN_ID[2], CAN_test_data_torque[count]);
    count++;
    if (count == 3){count = 0;}
  }
}