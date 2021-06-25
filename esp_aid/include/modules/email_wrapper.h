
#ifndef EMAIL_WRAPPER_H_
#define EMAIL_WRAPPER_H_

#include <Arduino.h>
#include <ESP_Mail_Client.h>

class EmailWrapper
{
public:
    EmailWrapper(const char* smtp_server, const int smtp_server_port, bool flag = true)
    : m_smtp_server(smtp_server), m_smtp_server_port(smtp_server_port), active_flag(flag) {}

    EmailWrapper() = delete;
    ~EmailWrapper() {}

    void setup() {
        if (!active_flag) {
          Serial.println("WARNING: Email is inactivated!");
        }

        smtp.debug(0);
        smtp.callback(sendCallback);
    }

    static void sendCallback(SMTP_Status status) {
        /* Print the current status */
        Serial.println(status.info());

        // Do something when complete
        if (status.success()) {
            Serial.println("----------------");
        }
    }

    void send(const char* sender_name, const char* sender_account, const char* sender_pass, 
              const int num_recipients, const char* recipients[], 
              char* subject, char* msg, char* filename, char* path, bool attach_file) {
        
        if (!active_flag) {
            return;
        }
        
        ESP_Mail_Session session;

        session.server.host_name = m_smtp_server;
        session.server.port = m_smtp_server_port;
        session.login.email = sender_account;
        session.login.password = sender_pass;

        SMTP_Message message;

        message.enable.chunking = true;
        message.sender.name = sender_name;
        message.sender.email = sender_account;
        message.subject = subject;

        // Add recipients, you can add more than one recipient
        char name[7];
        for (int i = 0; i < num_recipients; i++) {
            snprintf(name, 7, "User%d", i);
            message.addRecipient(name, recipients[i]);
        }

        message.html.content = msg;
        message.html.charSet = "utf-8";

        message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
        message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

        SMTP_Attachment att;
        att.descr.filename = filename;
        att.file.path = path;
        att.file.storage_type = esp_mail_file_storage_type_sd;

        message.addAttachment(att);

        if (!smtp.connect(&session)) {
            Serial.println("Unable to connect to smtp session!");
            return;
        }

        if (!MailClient.sendMail(&smtp, &message, true)) {
            Serial.println("Error sending Email, " + smtp.errorReason());
        }
    }


private:
    const char* m_smtp_server;
    int m_smtp_server_port;

    SMTPSession smtp;

    bool active_flag;
};

#endif