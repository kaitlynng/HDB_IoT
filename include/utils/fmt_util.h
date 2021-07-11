#ifndef FMT_UTIL_H_
#define FMT_UTIL_H_

#include <iostream>
#include <Arduino.h>

#include <ArduinoJson.h>
#include <WiFi.h>

DateTime tm2DateTime(struct tm t) {
    return DateTime(t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
}

void format_heartbeat_msg(float lat, float lon, const int cbuff_size, char* cbuff) {

    StaticJsonDocument<800> doc;
    JsonObject crane = doc.createNestedObject("crane");
    crane["lat"] = lat;
    crane["lon"] = lon;

    serializeJson(doc, cbuff, cbuff_size);
}

void format_job_msg(float lat, float lon, 
                     int pile_id, 
                     char* timestamp, float payload_lat, float payload_lon, 
                     char* sensor_status, char* gps_status,
                     int num_crane_fields, const char* fields[], const int tgt_ids[], float data[], 
                     const int cbuff_size, char* cbuff) {
                         
    StaticJsonDocument<800> doc;
    JsonObject crane = doc.createNestedObject("crane");
    crane["lat"] = lat;
    crane["lon"] = lon;

    JsonObject job = doc.createNestedObject("job");
    job["pile_id"] = pile_id;

    JsonObject payload = job.createNestedObject("payload");
    payload["timestamp"] = timestamp;
    payload["lat"] = payload_lat;
    payload["lon"] = payload_lon;

    payload["sensor_status"] = sensor_status;
    payload["gps_status"] = gps_status;

    
    for (int i = 0; i < num_crane_fields; i++) {
        payload[fields[i]] = data[tgt_ids[i]];
    }
    serializeJson(doc, cbuff, cbuff_size);
}

#endif