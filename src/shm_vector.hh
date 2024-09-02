#include "share_memory.hh"
#include <string_view>
#include <cstdint>
#include <cstdlib>

enum class ShMeError {
    MEMORYLEAK
};

template  <typename T>
class ShmArray {
 public:
    ShmArray(std::string_view file_path, int size, int mode):
        p(nullptr) {
        try {
            if (mode == 0)
            {
                createShareMemory(file_path, size);
            } else
            {

            }
        } catch (const std::exception& e) {
            exceptionHandle(e);
        }

    }

    bool emplace_back() {
        return true;
    }

    bool erase() {
        return true;
    }

 private:

    int64_t createShareMemory(std::string_view file_path, size_t size) {
        try {
            shm_fd.unlink();
            shm_fd.shm_open(file_path);
            shm_fd.ftruncate(size);
            p = static_cast<T*>(shm_fd.mmap(size));

        } catch (const std::exception& e) {
            exceptionHandle(e);
        }
    }

    int64_t linkShareMemory(std::string_view file_path) {
        shm_fd.shm_open(share_memory.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

        struct stat sb;
        if (fstat(shm_fd.native_handle(), &sb) == -1) {
            LOG("fstat is failed, error:", strerror(errno));
            close(shm_fd);
            exit(EXIT_FAILURE);
        }

        void* addr = mmap(nullptr, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd.native_handle(), 0);
        if (addr == MAP_FAILED) {
            LOG("mmap is failed, error:", strerror(errno));
            close(shm_fd);
            exit(EXIT_FAILURE);
        }
    
        p = static_cast<T*>(addr);

    if (close(shm_fd) == -1) {
        LOG("close is failed, error:", strerror(errno));
        exit(EXIT_FAILURE);
    }
    }

    void exceptionHandle(const std::exception& e) {
        LOG(e.what());
        exit(EXIT_FAILURE);
    }

    ShareMemory<T> shm_fd;
    T* p;
};

int main() {
    return 0;
}