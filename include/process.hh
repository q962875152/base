#ifndef PROCESS_HH_
#define PROCESS_HH_

#include <cstdint>
#include <sys/types.h>
#include <string>

struct FMBGS {
    uint16_t playfreq;
    uint16_t picode;
    uint8_t pty;
    uint8_t tp;
    uint8_t psname[8];
};

extern FMBGS* pfmbgs;
extern void *addr;
extern int shm_fd;
extern int* ptest;

void processParent(pid_t pid);
void processChildren();
void processChildren_test();
void configProcess();
void inputFromTerminal();
void createShareMemory(const std::string& share_memory);
void linkShareMemory(const std::string& share_memory);
void test_config();
#endif