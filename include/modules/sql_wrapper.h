#ifndef SQL_WRAPPER_H_
#define SQL_WRAPPER_H_

#include <Arduino.h>
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <TinyGsmClient.h>

class SqlWrapper
{
public:
    SqlWrapper(const char* hostname, const int port, const char* user, const char* pass, TinyGsmClient &client, bool flag = true) 
    :  m_port(port), conn(&client), active_flag(flag) {
          strncpy(m_hostname, hostname, 100);
          strncpy(m_user, user, 100);
          strncpy(m_pass, pass, 100);
      } //IPAddress serverAddr(216,219,81,80); 
    
    SqlWrapper() = delete;
    ~SqlWrapper() {}

    void setup() {
        if (!active_flag) {
            Serial.println("WARNING: SQL is inactivated!");
            return;
        }

        Serial.print("Connecting to SQL...  ");
        if (conn.connect(m_hostname, m_port, m_user, m_pass)) {
            Serial.println("OK.");
        } else {
            Serial.println("FAILED.");
            //sql_error = dateTime + " sql failed to sent "+nl;
            //sql_error.toCharArray(sql_error_sent, 100);
            //append_file(SD, "/error_log", " Connection FAILED \n");
        }
    }

    void insert(String data) {
        if (!active_flag) {
            return;
        }
        Serial.print("Connecting to SQL...  ");
        if (conn.connect(m_hostname, m_port, m_user, m_pass)) {
            Serial.println("OK.");
        } else {
            Serial.println("Failed to connect to server. Aborting...");
        }

        if (conn.connected()) {
            char data_c[800];  //SQL query as char array
            MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

            data.toCharArray(data_c, 800);
            cur_mem->execute(data_c);
            delete cur_mem;
            
            Serial.println("successfully inserted");

        } else {
            Serial.println("WARNING: SQL insert failed,");
            //append_file(SD, "/error_log", "SQL insert fail \n");
            //append_file(SD, "/sql_fail_sent",FSstorage);
        }
    }

private:
    char m_hostname[100];
    int m_port;
    char m_user[100];
    char m_pass[100];

    MySQL_Connection conn;

    bool active_flag;
};

#endif