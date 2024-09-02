#include "inout.hh"
#include <sys/socket.h>
#include <cerrno>

class SocketInt : public IO 
{
 public:
    SocketInt() 
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == fd) {
            // 输出log
        }
    }
 private:
    int fd;
};