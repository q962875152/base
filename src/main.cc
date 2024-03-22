#include "log.hh"
#include "message_queue.hh"
#include "command_line.hh"
#include "command_buffer.hh"
#include "threadpool.hh"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <thread>
#include "time_111.hh"
#include "../MWUtils/tool.hh"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/process.hh"

extern FMBGS* g_pfmbgs;

const std::string share_memory = "/radioservice";

pid_t pid = 0;
extern char **environ;
static std::string process_name;

void childProcess() {
    LOG("childProcess");
    char* args[] = {const_cast<char*>(process_name.c_str()), "run_workprocess", NULL};
    if (-1 == execvp(process_name.c_str(), args)) {
        LOG("execve is fail, error : ", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void clearProgram() {
    std::cout << "clearProgram, pid: " << getpid() << std::endl;
    munmap(static_cast<void*>(g_pfmbgs), 100 * sizeof(FMBGS));
    shm_unlink(share_memory.c_str());
}

static void monitorProcess() {
    atexit(clearProgram);
    while (true) {
        if (pid == 0) {
            std::this_thread::yield();
            continue;
        }
        if (-1 ==  waitpid(pid, nullptr, 0)) {
            LOG("waitpid is failed, error :", strerror(errno));
        }
        LOG("children process is EXIT, create children process again");
        switch (pid = fork()) {
            case -1: {
                LOG("fork is failed, error: ", strerror(errno));
                exit(EXIT_FAILURE);
                break;
            }
            case 0: {
                childProcess();
                // test_childProcess();  // test
                break;
            }
            default: {
                break;
            }
        }
    }
}

void parentProcess() {
    LOG("parentProcess");
    // create share_memory
    createShareMemory(share_memory);

    switch (pid = fork()) {
        case -1: {
            LOG("fork is failed, error: ", strerror(errno));
            exit(EXIT_FAILURE);
            break;
        }
        case 0: {
            childProcess();
            break;
        }
        default: {
            monitorProcess();
            break;
        }
    }
}

void workProcess() {
    LOG("workProcess");
    linkShareMemory(share_memory);
    // LOG("g_pfmbgs:", g_pfmbgs);
    test_config();

    std::this_thread::sleep_for(std::chrono::seconds(100));
}

int main(int argc, char** argv) {
    LOG("main is Start");
    process_name = argv[0];
    switch (argc) {
        case 1: {
            parentProcess();
            break;
        }
        case 2: {
            if (0 == std::strcmp(argv[1], "run_workprocess")) {
                workProcess();
            /*} else if (0 == std::strcmp(argv[1], "test_run_workprocess")) {
                test_workProcess();*/
            } else {
                LOG("cmd is failed");
            }
            break;
        }
        default: {
            exit(EXIT_FAILURE);
            break;
        }
    }
    LOG("main is end");
    return 0;
}

/***************************TEST***********************************/

void test_workProcess() {
    LOG("test_workProcess");
    linkShareMemory(share_memory);
    LOG("g_pfmbgs:", g_pfmbgs);
    for (int i = 0; i < 100; ++i) {
        LOG(g_pfmbgs[i].playfreq);
    }
    std::this_thread::sleep_for(std::chrono::seconds(100));
}

void test_childProcess() {
    LOG("test_childProcess");
    char* args[] = {const_cast<char*>(process_name.c_str()), "test_run_workprocess", NULL};
    if (-1 == execvp(process_name.c_str(), args)) {
        LOG("execve is fail, error : ", strerror(errno));
        exit(EXIT_FAILURE);
    }
}