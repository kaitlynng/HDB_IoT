#ifndef FMT_UTIL_H_
#define FMT_UTIL_H_

#include <iostream>
#include <Arduino.h>

#include <ArduinoJson.h>
#include <WiFi.h>

String br = "<br/>";
String nl = "\n";

DateTime tm2DateTime(struct tm t) {
    return DateTime(t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
}

void ip2String(IPAddress ip, char* cbuff) {
    sprintf(cbuff, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

String format_sql_msg(const String db, const String table, const int num_fields, const String fields[], const int tgt_ids[], String data[]) {
    String msg;
    String start = "INSERT INTO " + db + "." + table; //latti3,longi3
    String fields_string = "";
    String data_string = "";
    for (int i = 0; i < num_fields - 1; i++) {
        fields_string += fields[i] + ",";
        data_string += "'" + data[tgt_ids[i]] + "',";
    }
    fields_string += fields[num_fields - 1];
    data_string += "'" + data[tgt_ids[num_fields - 1]] + "'";
    msg = start + " (" + fields_string + ") VALUES (" + data_string + ");";
    return msg;
}

String format_csv_msg(int num_fields, const String fields[], const int tgt_ids[], String data[]) {
    String msg = "";
    for (int i = 0; i < num_fields - 1; i++) {
        msg += data[tgt_ids[i]] + ",";
    }

    msg += data[tgt_ids[num_fields - 1]] + nl;

    return msg;
}

String format_csv_header(int num_fields, const String fields[]) {
    String msg = "";
    for (int i = 0; i < num_fields - 1; i++) {
        msg += fields[i] + ",";
    }
    msg += fields[num_fields - 1] + nl;
    return msg;
}

String format_email_msg(int num_fields, const String fields[], const int tgt_ids[], String data[]) {
    String msg = "";
    
    for (int i = 0; i < 8; i++) {
        msg += fields[i] + ": " + data[tgt_ids[i]] + br;
    }

    msg += fields[8] + ": " + data[tgt_ids[8]] + ", " + data[tgt_ids[9]] + br; // lat and long has a special format

    for (int i = 10; i < num_fields; i++) {
        msg += fields[i] + ": " + data[tgt_ids[i]] + br;
    }

    return msg;
}

String format_json_msg(int num_fields, const String fields[], const int tgt_ids[], String data[]) {
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

    return msg;
}


#endif