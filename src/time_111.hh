#ifndef TIME_111_HH_
#define TIME_111_HH_

#include <stdint.h>
#include <thread>
#include <memory>
#include "../MWUtils/tool.hh"
#include <atomic>
#include <utility>
#include "../MWUtils/singleton.h"

class Timer;

class TimerMode : public Singleton<TimerMode> {
    friend Singleton<TimerMode>;
public:
    template  <typename F, typename... Args>
    uint64_t CreateTimerAt(int64_t when_ms, F&& f, Args... args);

    template  <typename F, typename... Args>
    uint64_t CreateTimerAfter(int64_t delay_ms, F&& f, Args... args);

    template  <typename F, typename... Args>
    uint64_t CreateTimerEvery(int64_t interval_ms, F&& f, Args... args);

    void CancelTimer(uint32_t timer_id);

    bool Start();

    void Stop();

    virtual ~TimerMode();

private:
    TimerMode();

    std::unique_ptr<Timer> TimerImpl;
};

#define  TimerProcess Singleton<TimerMode>::Instance()

void testTimer111();

#endif