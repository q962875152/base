#ifndef TOOL_HH_
#define TOOL_HH_

#include <iostream>
#include <iomanip>
#include <chrono>
#include <sys/time.h>

template <typename T>
inline void print(T&& value) {
    std::cout << value << std::endl;
}

template <typename T, typename... Args>
inline void print(T&& arg, Args&&... args) {
    std::cout << arg << " ";
    print(args...);
}

// use strftime to format time_t into a "date time"
inline std::string date_time(std::time_t posix) {
    char buf[20]; // big enough for 2015-07-08 10:06:51\0
    std::tm tp = *std::localtime(&posix);
    return {buf, std::strftime(buf, sizeof(buf), "%F %T", &tp)};
}

inline std::string getCurrentTime() {
    using namespace std;
    using namespace std::chrono;

    // get absolute wall time
    auto now = system_clock::now();

    // find the number of milliseconds
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // build output string
    std::ostringstream oss;
    oss.fill('0');

    // convert absolute time to time_t seconds
    // and convert to "date time"
    oss << date_time(system_clock::to_time_t(now));
    oss << '.' << setw(3) << ms.count();

    return oss.str();
}

#endif