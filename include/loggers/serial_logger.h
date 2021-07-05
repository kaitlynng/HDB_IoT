#ifndef SERIAL_LOGGER_H_
#define SERIAL_LOGGER_H_

#define PRINTF_BUF 128 // resulting string limited to 128 chars, modify if necessary

#define SERIAL_FMT_DEFAULT "\033[0;39;49m"
#define SERIAL_FMT_CYAN "\033[1;36m"
#define SERIAL_FMT_YELLOW "\033[1;33m"
#define SERIAL_FMT_RED "\033[1;31m"
#define SERIAL_FMT_RED_BG "\033[0;41m"

#include "interfaces/logger.h"
#include <cstdio>
#include <Arduino.h>

class SerialLogger : public Logger
{
public:
    SerialLogger(int log_level) : Logger(log_level) {}

    int log_debug_impl(const char *format, va_list arg) {
        char buf[PRINTF_BUF];
        vsnprintf(buf, PRINTF_BUF, format, arg);
        Serial.print(SERIAL_FMT_CYAN);
        Serial.print(buf);
        Serial.println(SERIAL_FMT_DEFAULT);
        return 1;
    }

    int log_info_impl(const char *format, va_list arg) {
        char buf[PRINTF_BUF];
        vsnprintf(buf, PRINTF_BUF, format, arg);
        Serial.println(buf);
        return 1;
    }

    int log_warn_impl(const char *format, va_list arg) {
        char buf[PRINTF_BUF];
        vsnprintf(buf, PRINTF_BUF, format, arg);
        Serial.print(SERIAL_FMT_YELLOW);
        Serial.print(buf);
        Serial.println(SERIAL_FMT_DEFAULT);
        return 1;
    }
    
    int log_error_impl(const char *format, va_list arg) {
        char buf[PRINTF_BUF];
        vsnprintf(buf, PRINTF_BUF, format, arg);
        Serial.print(SERIAL_FMT_RED);
        Serial.print(buf);
        Serial.println(SERIAL_FMT_DEFAULT);
        return 1;
    }

    int log_fatal_impl(const char *format, va_list arg) {
        char buf[PRINTF_BUF];
        vsnprintf(buf, PRINTF_BUF, format, arg);
        Serial.print(SERIAL_FMT_RED_BG);
        Serial.print(buf);
        Serial.println(SERIAL_FMT_DEFAULT);
        return 1;
    }
};

#endif