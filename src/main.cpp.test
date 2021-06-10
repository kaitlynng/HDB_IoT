#include "../config/system_config.h"
#include "../config/user_config.h"

#include <Arduino.h>
#include <string>
#include <time.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>

#include <ArduinoJson.h>

#include "modules/sd_wrapper.h"

char AWS_CERT_CA_FILENAME[50] =  "/AmazonRootCA1.pem";
char AWS_CERT_CRT_FILENAME[50] = "/893c5bb971-certificate.pem.crt";
char AWS_CERT_PRIVATE_FILENAME[50] =  "/893c5bb971-private.pem.key";

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(800);

const char aws_iot_endpoint[] = "a132xxzx3351g7-ats.iot.us-east-2.amazonaws.com";
const char device_name[] = "HDB6";
const char topic_name[] = "$aws/things/HDB6/shadow/update";

int aws_max_reconnect_tries = 50;

char aws_cert_ca[2000];
char aws_cert_crt[2000];
char aws_cert_private[2000];

SdWrapper sd_wrapper(SD_CS);

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(NETWORK_SSID, NETWORK_PASS);

  // Only try 15 times to connect to the WiFi
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 15){
    delay(500);
    Serial.print(".");
    retries++;
  }

  // If we still couldn't connect to the WiFi, go to deep sleep for a minute and try again.
  if(WiFi.status() != WL_CONNECTED){
    esp_sleep_enable_timer_wakeup(1 * 60L * 1000000L);
    esp_deep_sleep_start();
  }
}


void connectToAWS()
{
	// Configure WiFiClientSecure to use the AWS certificates we generated
	net.setCACert(aws_cert_ca);
	net.setCertificate(aws_cert_crt);
	net.setPrivateKey(aws_cert_private);

	// Connect to the MQTT broker on the AWS endpoint we defined earlier
	client.begin(aws_iot_endpoint, 8883, net);

	// Try to connect to AWS and count how many times we retried.
	int retries = 0;
	Serial.print("Connecting to AWS IOT");

	while (!client.connect(device_name) && retries < aws_max_reconnect_tries) {
		Serial.print(".");
		delay(100);
		retries++;
	}

	// Make sure that we did indeed successfully connect to the MQTT broker
	// If not we just end the function and wait for the next loop.
	if(!client.connected()){
		Serial.println(" Timeout!");
		return;
	}

	// If we land here, we have successfully connected to AWS!
	// And we can subscribe to topics and send messages.
	Serial.println("Connected!");
}

void sendJsonToAWS()
{
  StaticJsonDocument<512> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("state");
  JsonObject reportedObj = stateObj.createNestedObject("reported");
  
  // Write the temperature & humidity. Here you can use any C++ type (and you can refer to variables)
  reportedObj["temperature"] = 23.76;
  reportedObj["humidity"] = 78.12;
  reportedObj["wifi_strength"] = WiFi.RSSI();
  
  // Create a nested object "location"
  JsonObject locationObj = reportedObj.createNestedObject("location");
  locationObj["name"] = "Garden";
  
  Serial.println("Publishing message to AWS...");
  //serializeJson(doc, Serial);
  char jsonBuffer[512];
  serializeJson(jsonDoc, jsonBuffer);

  // Publish the message to AWS
  client.publish(topic_name, jsonBuffer);
}

void setup() {
  Serial.begin(115200);

  sd_wrapper.setup();

  sd_wrapper.read_file(AWS_CERT_CA_FILENAME, 2000, aws_cert_ca);
  sd_wrapper.read_file(AWS_CERT_CRT_FILENAME, 2000, aws_cert_crt);
  sd_wrapper.read_file(AWS_CERT_PRIVATE_FILENAME, 2000, aws_cert_private);

  connectToWiFi();
  connectToAWS();
}

void loop() {
  sendJsonToAWS();
  client.loop();
  delay(1000);
}