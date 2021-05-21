
#ifndef EMAIL_WRAPPER_H_
#define EMAIL_WRAPPER_H_

#include <Arduino.h>
#include <ESP32_MailClient.h>

class EmailWrapper
{
public:
    EmailWrapper(String smtp_server, int smtp_server_port, bool flag = true)
    : m_smtp_server(smtp_server), m_smtp_server_port(smtp_server_port), active_flag(flag) {}

    EmailWrapper() = delete;
    ~EmailWrapper() {}

    void setup() {
        if (!active_flag) {
          Serial.println("WARNING: Email is inactivated!");
        }
    }

    static void sendCallback(SendStatus msg) {
        // Print the current status
        Serial.println(msg.info());

        // Do something when complete
        if (msg.success()) {
            Serial.println("----------------");
        }
    }

    void send(const String sender_name, const String sender_account, const String sender_pass, 
              const int num_recipients, const String recipients[], 
              String subject, String msg, const String filename) {
        
        if (!active_flag) {
            return;
        }
        
        smtpData.setLogin(m_smtp_server, m_smtp_server_port, sender_account, sender_pass);
        
        // Set the sender name and Email
        smtpData.setSender(sender_name, sender_account);

        // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
        smtpData.setPriority("High");

        // Set the subject
        smtpData.setSubject(subject);

        // Set the message with HTML format
        smtpData.setMessage(msg, true);

        // Add recipients, you can add more than one recipient
        for (int i = 0; i < num_recipients; i++) {
          smtpData.addRecipient(recipients[i]);
        }

        smtpData.addAttachFile(filename);
        smtpData.setFileStorageType(MailClientStorageType::SD);

        smtpData.setSendCallback(sendCallback);

        //Start sending Email, can be set callback function to track the status
        if (!MailClient.sendMail(smtpData)) {
          Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
          //email_error = dateTime + " EMAIL "+ MailClient.smtpErrorReason()+nl;
          //email_error.toCharArray(email_error_sent, 100);
          //appendFile(SD, "/error_log", email_error_sent);
          //appendFile(SD, "/email_fail_sent",FSstorage);
        }

        smtpData.empty();
    }


private:
    String m_smtp_server;
    int m_smtp_server_port;

    SMTPData smtpData;

    bool active_flag;
};

#endif