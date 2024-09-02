#include "socket.hh"
#include <sys/socket.h>
#include <unistd.h>

// class UnixSocket : public Socket {
//  public:
//     UnixSocket() :
//         fd(-1) {
//         fd = socket
//     }
//     ~UnixSocket() {
//         close(fd);
//     }
//  private:
//     int fd;
// };