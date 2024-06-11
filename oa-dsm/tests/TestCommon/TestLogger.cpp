#include <gtest/gtest.h>
#include <iostream>
#include "../../DataStructure/Extent.h"
#include "../../Common/Logger.h"


TEST(Logger, BasicsLOG) {

    LOG(LoggerLevel::debug) << "My string" << " 1";
    LOG(LoggerLevel::info) << "My string" << " " << 2;
    LOG(LoggerLevel::warning) << "My string" << std::endl;
    LOG(LoggerLevel::error) << "My string";
    LOG(LoggerLevel::fatal) << "My string";
}

/*
TEST(Logger, BasicsStdCout) {
    BasicLogger bLogger;
    Logger& log = bLogger;
    log.LOG(LoggerLevel::debug) << "My string";
    log.LOG(LoggerLevel::info) << "My string";
    log.LOG(LoggerLevel::warning) << "My string";
    log.LOG(LoggerLevel::error) << "My string";
    log.LOG(LoggerLevel::fatal) << "My string";
}
*/
TEST(Logger, FilteringStdCout) {

    InitLogger(error, "log.txt");
    LOG(LoggerLevel::debug) << "My string";
    LOG(LoggerLevel::info) << "My string";
    LOG(LoggerLevel::warning) << "My string";
    LOG(LoggerLevel::error) << "My string";
    LOG(LoggerLevel::fatal) << "My string";
    Logger::Flush();
    std::ifstream log_f("log.txt");
    EXPECT_EQ(log_f.is_open(), true);
    std::string line;
    std::getline(log_f, line);
    EXPECT_EQ(line, "[error]    My string");
}

/*


TEST(Logger, FileLog) {
    {
        auto l = std::make_unique<FileLogger>("log.txt");

        l->LOG(LoggerLevel::debug) << "My string";
        l->LOG(LoggerLevel::info) << "My string";
        l->LOG(LoggerLevel::warning) << "My string";
        l->LOG(LoggerLevel::error) << "My string";
        l->LOG(LoggerLevel::fatal) << "My string";
    }
    std::ifstream log_f("log.txt");
    EXPECT_EQ(log_f.is_open(), true);
    std::string line;
    std::getline(log_f, line);
    EXPECT_EQ(line, "[debug]    My string");
}*/
/*
TEST(Logger, Polimorphism) {
    //FileLogger stdLogger("log.txt");
    Logger& LOG = std::make_shared<BasicLogger>(warning);

    LOG(LoggerLevel::debug) << "My string";
    LOG(LoggerLevel::info) << "My string";
    LOG(LoggerLevel::warning) << "My string";
    LOG(LoggerLevel::error) << "My string";
    LOG(LoggerLevel::fatal) << "My string";

    std::ifstream log_f("log.txt");
    EXPECT_EQ(log_f.is_open(), true);
    std::string line;
    std::getline(log_f, line);
    EXPECT_EQ(line, "[debug]    My string");
}*/