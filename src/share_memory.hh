#pragma once

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <string_view>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include "log.hh"
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <cstdlib>

class shm_error : public std::exception 
{
 public:
    shm_error(int err_code) :
        errno_(err_code)
    {}

    const char* what() const noexcept override {
        return strerror(errno_);
    }
 private:
    int errno_;
};

template <typename T>
class ShareMemory 
{
    constexpr static int SHFDCLOSE = -1;
 public:
    ShareMemory(std::string_view file_path, int mode) : 
        fd(SHFDCLOSE) 
    {
    }

    ~ShareMemory()
    {
        if (SHFDCLOSE != fd)  close(fd);
    }

    int createShareMemory(std::string_view file_path, int size) 
    {
        shm_path = file_path;
        fd = shm_open(file_path.data(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
        if ((fd == -1) && (errno != EEXIST)) {
            throw shm_error(errno);
        }

        if (ftruncate(fd, size * sizeof(T))) {
            throw shm_error(errno);
        }

        return 0;
    }

    int linkShareMemory(std::string file_path) {
        fd = shm_open(share_memory.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            LOG("shm_open is failed, error:", strerror(errno));
            throw shm_error(errno);
        }
    }

    int64_t size() {
        if (fd != SHFDCLOSE) {
            struct stat sb;
            if (fstat(fd, &sb) == -1) {
                throw shm_error(errno);
            }
            return sb.st_size;
        } else {
            return -1;
        }
    }

    void* mmap(size_t size) {
        void *addr = ::mmap(NULL, size * sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            throw shm_error(errno);
        }

        return addr;
    }

    bool unlink() {  // 这个函数需要调査下,写的对不对
        if (fd != SHFDCLOSE) {
            close(fd);
            fd = SHFDCLOSE;
        }

        shm_unlink(shm_path.c_str());
        return true;
    }

    int native_handle() {
        return fd;
    }

 private:
    int fd;
    std::string shm_path;
};