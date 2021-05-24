#ifndef SQL_WRAPPER_H_
#define SQL_WRAPPER_H_

#include <Arduino.h>
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

class SqlWrapper
{
public:
    SqlWrapper(const int server_addr[], const char* user, const char* pass, WiFiClient &client, bool flag = true) 
    : m_server_addr(server_addr[0], server_addr[1], server_addr[2], server_addr[3]), 
      conn(&client), active_flag(flag) {
          m_user = strdup(user); // convert const char* to char* for SQL API calls
          m_pass = strdup(pass);
      } //IPAddress serverAddr(216,219,81,80); 

    SqlWrapper() = delete;
    ~SqlWrapper() {}

    void setup() {
        if (!active_flag) {
            Serial.println("WARNING: SQL is inactivated!");
            return;
        }

        Serial.print("Connecting to SQL...  ");
        if (conn.connect(m_server_addr, 3306, m_user, m_pass)) {
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
        if (conn.connect(m_server_addr, 3306, m_user, m_pass)) {
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
    char* m_user;
    char* m_pass;
    IPAddress m_server_addr;

    WiFiClient client;
    MySQL_Connection conn;

    bool active_flag;
};

#endif