#include "inout.hh"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unistd.h>

class Terminal : public IO 
{
 public:
    Terminal() :
        write_fd(STDOUT_FILENO),
        read_fd(STDIN_FILENO)
    {}

    int32_t read(void* buf, size_t count) override {
        ssize_t length = ::read(read_fd, buf, count);
        // 异常处理
        return length;
    }

    int32_t write(const void* buf, size_t count) override 
    {
        ssize_t length = ::write(write_fd, buf, count); 
        // 异常处理
        return length;
    }

 private:
    int write_fd;
    int read_fd;
};