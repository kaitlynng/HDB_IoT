#ifndef FMT_UTIL_H_
#define FMT_UTIL_H_

#include <iostream>
#include <Arduino.h>

#include <ArduinoJson.h>
#include <WiFi.h>


// void strncat_fast(char* dest, char* src, char* last, size_t n) {
//     while (*dest) dest++;
//     while (*dest++ = *src++);
//     return --dest;
// }

DateTime tm2DateTime(struct tm t) {
    return DateTime(t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
}

void ip2String(IPAddress ip, char* cbuff) {
    sprintf(cbuff, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

int format_blast_msg(const int num_fields, char (*data)[50], const int cbuff_size, char* cbuff) {
    int cx = 0;
    for (int i = 0; i < num_fields; i++) {
        cx += snprintf(cbuff + cx, cbuff_size - cx, "%s,", data[i]);
    }

    cx += snprintf(cbuff + cx, cbuff_size - cx, "\n");

    return cx;
}

int format_sql_msg(const char* db, const char* table, 
                    const int num_fields, const char* fields[], const int tgt_ids[], char (*data)[50], 
                    const int cbuff_size, char* cbuff) {
    int cx;

    cx = snprintf(cbuff, cbuff_size, "INSERT INTO %s.%s (%s", db, table, fields[0]);

    for (int i = 1; i < num_fields; i++) {
        cx += snprintf(cbuff+cx, cbuff_size-cx, ",%s", fields[i]);
    }

    cx += snprintf(cbuff+cx, cbuff_size-cx, ") VALUES ('%s'", data[tgt_ids[0]]);

    for (int i = 1; i < num_fields; i++) {
        cx += snprintf(cbuff+cx, cbuff_size-cx, ",'%s'", data[tgt_ids[i]]);
    }

    cx += snprintf(cbuff+cx, cbuff_size-cx, ");");

    return cx;
}

int format_csv_msg(const int num_fields, const char* fields[], const int tgt_ids[], char (*data)[50], 
                    const int cbuff_size, char* cbuff) {
    int cx = 0;
    for (int i = 0; i < num_fields - 1; i++) {
        cx += snprintf(cbuff + cx, cbuff_size - cx, "%s,", data[tgt_ids[i]]);
    }

    cx += snprintf(cbuff + cx, cbuff_size - cx, "%s\n", data[tgt_ids[num_fields - 1]]);

    return cx;
}

int format_csv_header(int num_fields, const char* fields[], const int cbuff_size, char* cbuff) {
    int cx = 0;
    for (int i = 0; i < num_fields - 1; i++) {
        cx += snprintf(cbuff + cx, cbuff_size - cx, "%s,", fields[i]);
    }

    cx += snprintf(cbuff + cx, cbuff_size - cx, "%s\n", fields[num_fields - 1]);

    return cx;
}

int format_email_msg(const int num_fields, const char* fields[], const int tgt_ids[], char (*data)[50],
                      const int cbuff_size, char* cbuff) {
    
    int cx = 0;
    for (int i = 0; i < 8; i++) {
        cx += snprintf(cbuff + cx, cbuff_size - cx, "%s: %s<br/>", fields[i], data[tgt_ids[i]]);
    }

    cx += snprintf(cbuff + cx, cbuff_size - cx, "%s: %s, %s<br/<", fields[8], data[tgt_ids[8]], data[tgt_ids[9]]); // lat and lon special format
    
    for (int i = 10; i < num_fields; i++) {
        cx += snprintf(cbuff + cx, cbuff_size - cx, "%s: %s<br/>", fields[i], data[tgt_ids[i]]);
    }

    return cx;
}

void format_json_msg(int num_fields, const char* fields[], const int tgt_ids[], char (*data)[50], //TODO
                     const int cbuff_size, char* cbuff) {
    StaticJsonDocument<800> doc;
    String msg;

    doc[fields[0]] = data[tgt_ids[0]];
    doc[fields[1]] = data[tgt_ids[1]];
    doc[fields[2]][0] = data[tgt_ids[2]];
    doc[fields[2]][1] = data[tgt_ids[3]]; //lat and longi is special

    for (int i = 4; i < 7; i++) {
        doc[fields[i]] = data[tgt_ids[i]];
    }

    JsonObject parameters = doc.createNestedObject("Parameters");
    for (int i = 7; i < num_fields; i++) {
        parameters[fields[i]] = data[tgt_ids[i]];
    }

    serializeJsonPretty(doc, msg);

    msg.toCharArray(cbuff, cbuff_size);
}

#endif