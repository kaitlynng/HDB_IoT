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

    String read_file(String filename) {
        String SD_saved;
        File file = SD.open(filename);
        while (file.available()) {
            SD_saved = file.readString();
        }
        file.close();
        Serial.println("read sucessful");
        return SD_saved;
    }

    void write_file(char* path, String data) {
        File file = SD.open(path, FILE_WRITE);
        if (!file) {
            Serial.println("WARNING: Failed to open file for writing");
            return;
        }
        if (!file.print(data)) {
            Serial.println("WARNING: Failed to write to file!");
        }
        file.close();
    }

    void append_file(String path, String data) {
        File file = SD.open(path, FILE_APPEND);
        if (!file) {
            Serial.println("WARNING: Failed to open file for writing");
            return;
        }

        if (!file.print(data)) {
            Serial.println("WARNING: Failed to write to file!");
        }
        file.close();
    }

private:
    const int m_sd_cs;
    SPIClass spiSD;
};



#endif