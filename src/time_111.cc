#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <stdint.h>
#include <functional>
#include <map>
#include <memory>
#include <chrono>
#include <sys/select.h>
#include <thread>
#include "../MWUtils/tool.hh"
#include "log.hh"
#include "time_111.hh"

class Task {
public:
    virtual void run() = 0;
    virtual ~Task() {};
};

template  <typename F, typename... Args>
class TimerTask : public Task {
public:
    TimerTask(F&& f, Args &&... args) :
        func([=]() {
            std::invoke(f, args...);}) {
    }
    void run() override {
        func();
    }
private:
    std::function<void()> func;
};

class TimerTaskWrapper {
public:
    TimerTaskWrapper(uint64_t target_ms, uint32_t interval_ms, std::shared_ptr<Task> pTimerTask) :
        target_ms_(target_ms),
        interval_ms_(interval_ms),
        pTimerTask_(pTimerTask),
        timer_task_id_(++count) {
    }
    uint64_t getTargetMs() {
        return target_ms_;
    }
    uint32_t getIntervalMs() {
        return interval_ms_;
    }
    uint64_t getTimerId() {
        return timer_task_id_;
    }
    void operator()() {
        pTimerTask_->run();
    }
    bool checkCountAtomic() {
        return std::atomic_uint64_t::is_always_lock_free;
    }
private:
    uint64_t target_ms_;
    uint32_t interval_ms_;
    std::shared_ptr<Task> pTimerTask_;
    uint64_t timer_task_id_;
    static std::atomic_uint64_t count;
};

std::atomic_uint64_t TimerTaskWrapper::count = 0;

using TaskType = std::shared_ptr<TimerTaskWrapper>;

class Timer {
public:
    Timer() :
        is_stop_(false) {
    }
    template  <typename F, typename... Args>
    uint64_t CreateTimerAt(int64_t when_ms, F&& f, Args &&... args) {
        return setTimerTask(std::make_shared<TimerTaskWrapper>(when_ms, 0, std::make_shared<TimerTask<F, Args...>>(f, args...)));
    }
    template  <typename F, typename... Args>
    uint64_t CreateTimerAfter(int64_t delay_ms, F&& f, Args &&... args) {
        uint64_t target_ms = GetNowTimestamp() + delay_ms;
        return setTimerTask(std::make_shared<TimerTaskWrapper>(target_ms, 0, std::make_shared<TimerTask<F, Args...>>(f, args...)));
    }
    template  <typename F, typename... Args>
    uint64_t CreateTimerEvery(int64_t interval_ms, F&& f, Args &&... args) {
        uint64_t target_ms = GetNowTimestamp() + interval_ms;
        return setTimerTask(std::make_shared<TimerTaskWrapper>(target_ms, interval_ms, std::make_shared<TimerTask<F, Args...>>(f, args...)));
    }

    virtual const TaskType getFirstTimerTask() const = 0;
    virtual void popFirstTimerTask() = 0;
    virtual void CancelTimer(uint32_t timer_id) = 0;

    bool Start() {
        try {
            handler = std::thread([this] {
                this->run();
            });
            return true;
        } catch (...) {
            print("Timer happen a error");
            return false;
        }
    }

    void Stop() {
        is_stop_ = true;
    };

    virtual ~Timer() {
        print("Timer is Start deconstruct");
        Stop();
        if (handler.joinable()) {
            handler.join();
        }
        print("Timer is end deconstruct");
    };

protected:
    bool isStop() {
        return is_stop_;
    }

    uint64_t GetNowTimestamp() {
        using namespace std::chrono;
        auto now = system_clock::now().time_since_epoch();
        return duration_cast<milliseconds>(now).count();
    }

    virtual uint32_t setTimerTask(const std::shared_ptr<TimerTaskWrapper> timerTask) = 0;
    virtual void run() = 0;
    std::thread handler;
private:
    std::atomic_bool is_stop_;
};

class Timer_RBTree : public Timer {
public:
    Timer_RBTree() :
        last_target_ms_(UINT64_MAX),
        timer_step_ms_(2) {
    }

    const TaskType getFirstTimerTask() const override {
        std::lock_guard<std::mutex> lck(mtx);
        if (rb_tree.empty()) {
            return nullptr;
        }

        auto iter = rb_tree.cbegin();
        return iter->second;
    }

    void popFirstTimerTask() override {
        std::lock_guard<std::mutex> lck(mtx);
        if (rb_tree.empty()) {
            last_target_ms_ = UINT64_MAX;
        } else {
            auto iter = rb_tree.cbegin();
            rb_tree.erase(iter);
            if (rb_tree.empty()) {
                last_target_ms_ = UINT64_MAX;
            } else {
                auto iter = rb_tree.cbegin();
                last_target_ms_ = iter->first;
            }
        }
    }

    void CancelTimer(uint32_t timer_id) override {
        std::lock_guard<std::mutex> lck(mtx);
        if (auto iter = rb_tree.find(timer_id) != rb_tree.cend()) {
            rb_tree.erase(iter);
        }
    }
    ~Timer_RBTree() {
        print("Timer_RBTree is deconstruct");
    }
protected:
    uint32_t setTimerTask(const TaskType pTimerTask) override {
        uint64_t target_ms = pTimerTask->getTargetMs();
        LOG("setTimerTask target_ms: ", target_ms);
        std::lock_guard<std::mutex> lck(mtx);
        if ((target_ms < last_target_ms_)) {
            last_target_ms_ = target_ms;
        }
        rb_tree.emplace(target_ms, pTimerTask);
        return pTimerTask->getTimerId();
    }

    void run() override {
        LOG("Run is Running!");
        while (true) {
            if (isStop()) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(timer_step_ms_));
            uint64_t current_time = GetNowTimestamp();
            // LOG("current_time: ", current_time, " last_target_ms_: ", last_target_ms_);
            if (current_time < last_target_ms_) {
                continue;
            }

            while (current_time >= last_target_ms_) {
                LOG("current_time: ", current_time, " last_target_ms_: ", last_target_ms_);
                auto task = getFirstTimerTask();
                if (task == nullptr) {
                    LOG("error is happened!");
                    continue;
                }
                popFirstTimerTask();
                (*task)();
            }
        }
        LOG("Run is end!");
    }

private:
    std::map<uint64_t, TaskType> rb_tree;
    uint64_t last_target_ms_;
    mutable std::mutex mtx;
    uint16_t timer_step_ms_;
};

class RbtreeTimerFactory {
public:
    std::unique_ptr<Timer> create();
};

std::unique_ptr<Timer> RbtreeTimerFactory::create() {
    return std::make_unique<Timer_RBTree>();
}

TimerMode::TimerMode() {
    auto factory = std::make_unique<RbtreeTimerFactory>();
    TimerImpl = factory->create();
}

template  <typename F, typename... Args>
uint64_t TimerMode::CreateTimerAt(int64_t when_ms, F&& f, Args... args) {
    return TimerImpl->CreateTimerAt(when_ms, std::forward<F>(f), std::forward<Args>(args)...);
}

template  <typename F, typename... Args>
uint64_t TimerMode::CreateTimerAfter(int64_t delay_ms, F&& f, Args... args) {
    return TimerImpl->CreateTimerAfter(delay_ms, std::forward<F>(f), std::forward<Args>(args)...);
}

template  <typename F, typename... Args>
uint64_t TimerMode::CreateTimerEvery(int64_t interval_ms, F&& f, Args... args) {
    return TimerImpl->CreateTimerEvery(interval_ms, std::forward<F>(f), std::forward<Args>(args)...);
}

void TimerMode::CancelTimer(uint32_t timer_id) {
    TimerImpl->CancelTimer(timer_id);
}

bool TimerMode::Start() {
    return TimerImpl->Start();        
}

void TimerMode::Stop() {
    TimerImpl->Stop();
};

TimerMode::~TimerMode() = default;





/************************TEST*****************************/

void test1() {
    LOG("test1 is Start");
}

void test2() {
    LOG("test2 is Start");
}

void testTimer111() {
    LOG("testTimer111 is Start");
    TimerProcess.Start();
    TimerProcess.CreateTimerAfter(5000, test1);
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));
    LOG("testTimer111 is End");
}