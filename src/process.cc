#include "../include/process.hh"
#include "../include/config.hh"
#include "log.hh"
#include "iostream"
#include <chrono>
#include <cstdlib>
#include <sched.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include "sys/mman.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <strstream>
#include <vector>

FMBGS* g_pfmbgs = nullptr;

void configProcess() {
    ConfigProcess.init(config_file_path);
    ConfigProcess.print_test();
}

void test_config() {
    ConfigProcess.init(config_file_path);
    ConfigProcess.print_test();
    ConfigProcess.update_config();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    LOG("setRssi");
    ConfigProcess.setRssi(65);
    ConfigProcess.saveToFile();
    ConfigProcess.print_test();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    LOG("setSnr");
    ConfigProcess.setSnr(15);
    ConfigProcess.saveToFile();
}

void inputFromTerminal() {
    std::string cmd;
    while (std::getline(std::cin, cmd)) {
        std::istringstream istr(cmd);
        std::vector<std::string> word;
        std::string str;
        while (istr >> str) {
            word.emplace_back(str);
        }
        if (word.size() == 3) {
            if (word[0] == "setConfig") {
                if (word[1] == "rssi") {
                    ConfigProcess.setRssi(std::stoi(word[2]));
                } else if (word[1] == "snr") {
                    ConfigProcess.setSnr(std::stoi(word[2]));
                } else if (word[1] == "usn") {
                    ConfigProcess.setUsn(std::stoi(word[2]));
                }
            }
        }
    }
}

/********************parent-children process model**************************/

void createShareMemory(const std::string& share_memory) {
    int shm_fd = shm_open(share_memory.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if ((shm_fd == -1) && (errno != EEXIST)) {
        std::cout << "shm_open failed, errno:" << errno << ": " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } else if ((shm_fd == -1) && (errno == EEXIST)) {
        shm_unlink(share_memory.c_str());  // 如果直接用会怎么样
        shm_fd = shm_open(share_memory.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
        if (shm_fd == -1) {
            std::cout << "shm_open failed, errno:" << errno << ": " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if (ftruncate(shm_fd, 100 * sizeof(FMBGS)) == -1) {
        std::cout << "ftruncate failed, errno:" << errno << ": " << strerror(errno) << std::endl;
        close(shm_fd);
        shm_unlink(share_memory.c_str());
        exit(EXIT_FAILURE);
    }

    void *addr = mmap(NULL, 100 * sizeof(FMBGS), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        std::cout << "mmap failed, errno:" << errno << ": " << strerror(errno) << std::endl;
        close(shm_fd);
        shm_unlink(share_memory.c_str());
        exit(EXIT_FAILURE);
    }
    if (close(shm_fd) == -1) {
        LOG("close is failed");
        exit(EXIT_FAILURE);
    }
    g_pfmbgs = static_cast<FMBGS*>(addr);

    // LOG("g_pfmbgs:", g_pfmbgs);
}

void linkShareMemory(const std::string& share_memory) {
    int shm_fd = shm_open(share_memory.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        LOG("shm_open is failed, error:", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct stat sb;
    if (fstat(shm_fd, &sb) == -1) {
        LOG("fstat is failed, error:", strerror(errno));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    void* addr = mmap(nullptr, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        LOG("mmap is failed, error:", strerror(errno));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    g_pfmbgs = static_cast<FMBGS*>(addr);

    if (close(shm_fd) == -1) {
        LOG("close is failed, error:", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/*****************************************************************************/