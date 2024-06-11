#include <vector>
#include <string>
#include <mutex>
#include <iomanip>

#include "Logger.h"

std::mutex mtx;

class LoggerSettings {
public:
    LoggerLevel minSeverityLevel;
};

static LoggerSettings loggerSettings;

LoggerLevel GetMinSeverity() {
    return loggerSettings.minSeverityLevel;
}

inline std::string SeverityTable(LoggerLevel level){
    std::string levelString;
    switch(level){
        case debug:
            levelString = "[debug]    ";
            break;
        case info:
            levelString = "[info]     ";
            break;
        case warning:
            levelString = "[warning]  ";
            break;
        case error:
            levelString = "[error]    ";
            break;
        case fatal:
            levelString = "[fatal]    ";
            break;
    }
    return levelString;
}

Logger LOG(LoggerLevel level) {
    return Logger(level);
}

void InitLogger(LoggerLevel minLoggingLevel, std::string_view filePath){
    loggerSettings.minSeverityLevel = minLoggingLevel;
}

void Logger::Flush(){
}

Logger::~Logger() {

    if (severity >= loggerSettings.minSeverityLevel) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::unique_lock<std::mutex> logLock(mtx);
        std::cout << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %T] ") << SeverityTable(severity) << buffer.str()
                  << std::endl << std::flush; // << std::endl;
    }
    buffer.clear();
}