#ifndef CONSENSUS_LOGGER_H
#define CONSENSUS_LOGGER_H

#include <iostream>
#include <fstream>
#include <memory>
#include <string_view>
#include <sstream>

enum LoggerLevel{
    debug,
    info,
    warning,
    error,
    fatal
};


class Logger;
Logger LOG(LoggerLevel level);
void InitLogger(LoggerLevel minLevel = LoggerLevel::debug, std::string_view filePath = "");

LoggerLevel GetMinSeverity();

class Logger {
    friend void InitLogger(LoggerLevel minLevel, std::string_view filePath);
    //friend void LOG(LoggerLevel level, std::string_view msg);
    //std::ofstream lSink;
protected:
//    LoggerLevel minSeverityLevel = debug;
    LoggerLevel severity = info;
    std::stringstream buffer;

public:
    Logger() = default;
    explicit Logger(LoggerLevel level) : severity(level) {};

    template <typename T>
    Logger& operator<<(const T& msg) {
        if (severity >= GetMinSeverity()) {
            //std::stringstream s;
            //s << msg;
            buffer << msg;//s.str();
        }
        return *this;
    }
    Logger& operator<<(std::ostream& (*msg)(std::ostream&)) {
        if (severity >= GetMinSeverity()) {
            //std::stringstream s;
            //s << msg;
            buffer << msg;//s.str();
        }
        return *this;
    }

    static void Flush();
    ~Logger();
};
#endif //CONSENSUS_LOGGER_H
