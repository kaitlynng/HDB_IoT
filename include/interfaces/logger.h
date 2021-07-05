#ifndef LOGGER_H_
#define LOGGER_H_

#include <cstdarg>

enum LogLevel {LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG};

class Logger
{
public:
    Logger(int log_level) {
        m_level = log_level;
    }

    virtual ~Logger() {}

    int log_debug(const char *format, ...) {
        va_list arg;
        int done = 0;

        va_start(arg, format);
        done = vlog_debug(format, arg);
        va_end(arg);

        return done;
    }

    int vlog_debug(const char *format, va_list arg) {
        int done = 0;

        if (m_level >= LogLevel::LOG_DEBUG) {
            done = log_debug_impl(format, arg);
        }

        return done;
    }

    int log_info(const char *format, ...) {
        va_list arg;
        int done = 0;

        va_start(arg, format);
        done = vlog_info(format, arg);
        va_end(arg);

        return done;
    }

    int vlog_info(const char *format, va_list arg) {
        int done = 0;

        if (m_level >= LogLevel::LOG_INFO) {
            done = log_info_impl(format, arg);
        }

        return done;
    }

    int log_warn(const char *format, ...) {
        va_list arg;
        int done = 0;

        va_start(arg, format);
        done = vlog_warn(format, arg);
        va_end(arg);

        return done;

    }

    int vlog_warn(const char *format, va_list arg) {
        int done = 0;

        if (m_level >= LogLevel::LOG_WARN) {
            done = log_warn_impl(format, arg);
        }

        return done;
    }

    int log_error(const char *format, ...) {
        va_list arg;
        int done = 0;

        va_start(arg, format);
        done = vlog_error(format, arg);
        va_end(arg);

        return done;

    }

    int vlog_error(const char *format, va_list arg) {
        int done = 0;

        if (m_level >= LogLevel::LOG_ERROR) {
            done = log_error_impl(format, arg);
        }

        return done;
    }

    int log_fatal(const char *format, ...) {
        va_list arg;
        int done = 0;

        va_start(arg, format);
        done = vlog_fatal(format, arg);
        va_end(arg);

        return done;
    }

    int vlog_fatal(const char *format, va_list arg) {
        int done = 0;

        if (m_level >= LogLevel::LOG_FATAL) {
            done = log_fatal_impl(format, arg);
        }

        return done;
    }

    virtual int log_debug_impl(const char *format, va_list arg);
    virtual int log_info_impl(const char *format, va_list arg);
    virtual int log_warn_impl(const char *format, va_list arg);
    virtual int log_error_impl(const char *format, va_list arg);
    virtual int log_fatal_impl(const char *format, va_list arg);

private:
    int m_level;
    
};

#endif