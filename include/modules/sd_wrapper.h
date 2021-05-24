#ifndef SD_WRAPPER_H_
#define SD_WRAPPER_H_

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <SD.h>


class SdWrapper
{
public:
    SdWrapper(const int sd_cs) : m_sd_cs(sd_cs), spiSD(HSPI) {}

    SdWrapper()  = delete;
    ~SdWrapper() {}

    void setup() {
        spiSD.begin(14, 27, 13, m_sd_cs); //SCK,MISO,MOSI,SS //HSPI1
        if (!SD.begin(m_sd_cs, spiSD )) {
            Serial.println("WARNING: Card Mount Failed");
            return;
        }

        if (SD.cardType() == CARD_NONE) {
            Serial.println("WARNING: No SD card attached");
            return;
        }
    }

    bool is_file_available(char* path) {
        File file = SD.open(path);
        return file.available();
    }

    bool read_file(char* path, const int cbuff_size, char* cbuff) {
        String data_s;
        Serial.print("Reading from SD: ");
        Serial.print(path);
        Serial.print(".....");
        File file = SD.open(path);
        if (!file.available()) {
            Serial.println("WARNING: Unable to open file!");
            return false;
        }
        while (file.available()) {
            data_s = file.readString().trim();
        }
        file.close();
        if (data_s) {
            data_s.toCharArray(cbuff, cbuff_size);
            Serial.println("read sucessful");
            return true;
        } else {
            Serial.println("WARNING: Read failed!");
            return false;
        }
        
        
    }

    void write_file(char* path, char* data) {
        File file = SD.open(path, FILE_WRITE);
        String data_s = data;
        if (!file) {
            Serial.println("WARNING: Failed to open file for writing");
            return;
        }
        if (!file.print(data_s)) {
            Serial.println("WARNING: Failed to write to file!");
        }
        file.close();
    }

    void append_file(char* path, char* data) {
        File file = SD.open(path, FILE_APPEND);
        String data_s = data;
        if (!file) {
            Serial.println("WARNING: Failed to open file for writing");
            return;
        }

        if (!file.print(data_s)) {
            Serial.println("WARNING: Failed to write to file!");
        }
        file.close();
    }

private:
    const int m_sd_cs;
    SPIClass spiSD;
};



#endif