#include "log.hh"
#include <iostream>

void Log::init() {
    // log output to terminal
}

void Log::init(const std::string& filename) {
    // coding
}

void Log::output() {
    if (file.is_open()) {
        file << log.str() << std::endl;
    } else {
        std::cout << log.str() << std::endl;
    }
    log.str("");
}

void LogTest() {
    LOG("sdfs", 555, "dsfsf");
    LOG("This thread has been created!");
}