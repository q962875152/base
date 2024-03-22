#ifndef LOG_HH_
#define LOG_HH_

#include "../MWUtils/singleton.h"
#include "../MWUtils/tool.hh"
#include <mutex>
#include <string>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <thread>
#include <sys/types.h>
#include <unistd.h>

class Log : public Singleton<Log> {
    friend Singleton<Log>;
public:
    void init();
    void init(const std::string& filename);  // 后续添加输出log到文件中的功能

    void print() {
        std::lock_guard<std::recursive_mutex> lck(rmtx);
        std::cout << std::endl;
    }

    template <typename T>
    void print(T&& value) {
        std::lock_guard<std::recursive_mutex> lck(rmtx);
        log << value;
        output();
    }

    template <typename T, typename... Args>
    inline void print(T&& arg, Args&&... args) {
        std::lock_guard<std::recursive_mutex> lck(rmtx);
        log << arg << " ";
        print(args...);
    }

    void output();

private:
    Log() = default;
private:
    std::ostringstream log;
    std::ofstream file;
    std::recursive_mutex rmtx;
};

#define LOG(...) Singleton<Log>::Instance().print(getCurrentTime(), getpid(), \
                                    std::this_thread::get_id(), __LINE__, __VA_ARGS__)

void LogTest();

#endif