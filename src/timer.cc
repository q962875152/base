// #include "timer.hh"
#include "../MWUtils/tool.hh"
#include <vector>
#include <string>
#include <list>
#include <memory>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <future>

class Timer;
using TimerPtr = std::shared_ptr<Timer>;
using TimerTask = std::function<void()>;

class Timer {
public:
    Timer(uint32_t id, int64_t when_ms, int64_t interval_ms, const TimerTask& task);
    void Run();
    uint32_t id() const {
        return id_;
    }
    int64_t when_ms() const {
        return when_ms_;
    }
    bool repeated() const {
        return repeated_;
    }
    void UpdateWhenTime() {
        when_ms_ += interval_ms_;
    }
private:
    uint32_t id_;
    TimerTask task_;
    int64_t when_ms_;  // 生成定时任务时的时间
    uint32_t interval_ms_;  // 轮询任务的事件间隔
    bool repeated_;  // 是否是轮询任务
};

Timer::Timer(uint32_t id, int64_t when_ms, int64_t interval_ms, const TimerTask& task) :
    id_(id),
    task_(task),
    when_ms_(when_ms),
    interval_ms_(interval_ms),
    repeated_(interval_ms > 0) {
}

void Timer::Run() {
    if (task_) {
        task_();
    }
}

class TimeWheel {
public:
    TimeWheel(uint32_t scales, uint32_t scale_uint_ms, const std::string& name = "");

    uint32_t scale_unit_ms() const {
        return  scale_unit_ms_;
    }

    uint32_t scales() const {
        return scales_;
    }

    uint32_t current_index() const {
        return current_index_;
    }

    void set_less_level_tw(TimeWheel* less_level_tw) {
        less_level_tw_ = less_level_tw;
    }

    void set_greater_level_tw(TimeWheel* greater_level_tw) {
        greater_level_tw_ = greater_level_tw;
    }

    int64_t GetCurrentTime() const;

    void AddTimer(TimerPtr timer);

    void Increase();

    std::list<TimerPtr> GetAndClearCurrentSlot();

private:
    std::string name_;
    uint32_t current_index_;

    // A time wheel can be devided into multiple scales. A scals has N ms.
    uint32_t scales_;
    uint32_t scale_unit_ms_;

    // Every slot corresponds to a scale. Every slot contains the timers.
    std::vector<std::list<TimerPtr>> slots_;

    TimeWheel* less_level_tw_;
    TimeWheel* greater_level_tw_;
};

using TimeWheelPtr = std::shared_ptr<TimeWheel>;

inline int64_t GetNowTimestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(now).count();
}

TimeWheel::TimeWheel(uint32_t scales, uint32_t scale_unit_ms, const std::string& name) :
    name_(name),
    current_index_(0),
    scales_(scales),
    scale_unit_ms_(scale_unit_ms),
    slots_(scales),
    less_level_tw_(nullptr),
    greater_level_tw_(nullptr) {
}

int64_t TimeWheel::GetCurrentTime() const {
    int64_t time = current_index_ * scale_unit_ms_;
    if (less_level_tw_ != nullptr)
    {
        time += less_level_tw_->GetCurrentTime();
    }

    return time;
}

void TimeWheel::AddTimer(TimerPtr timer) {
    int64_t less_tw_time = 0;
    if (less_level_tw_ != nullptr)
    {
        less_tw_time = less_level_tw_->GetCurrentTime();
    }
    int64_t diff = timer->when_ms() + less_tw_time - GetNowTimestamp();

    // if the difference is greater than scale unit, the timer can be added into the current time wheel.
    if (diff >= scale_unit_ms_)
    {
        size_t n = (current_index_ + diff / scale_unit_ms_) % scales_;
        slots_[n].push_back(timer);
        return;
    }

    // If the difference is less than scale uint, the timer should be added into less level time wheel.
    if (less_level_tw_ != nullptr) {
        less_level_tw_->AddTimer(timer);
        return;
    }

    // If the current time wheel is the least level, the timer can be added into the current time wheel.
    slots_[current_index_].push_back(timer);
}

void TimeWheel::Increase() {
    // Increase the time wheel
    ++current_index_;
    if (current_index_ < scales_) {
        return;
    }

    // If the time wheel is full, the greater level time wheel should be increased.
    // The timers in the current slot of the greater level time wheel should be moved into
    // the less level time wheel.
    current_index_ = current_index_ % scales_;
    if (greater_level_tw_ != nullptr) {
        greater_level_tw_->Increase();
        std::list<TimerPtr> slot = greater_level_tw_->GetAndClearCurrentSlot();
        for (auto timer : slot) {
            AddTimer(timer);
        }
    }
}

std::list<TimerPtr> TimeWheel::GetAndClearCurrentSlot() {  // 将事件向下转移
    std::list<TimerPtr> slot;
    slot = std::move(slots_[current_index_]);
    return slot;
}

//  time_wheel_scheduler
class TimeWheelScheduler {
public:
    explicit TimeWheelScheduler(uint32_t timer_step_ms = 50);
    ~TimeWheelScheduler();

    // return timer id. return 0 if the timer creation fails.
    uint32_t CreateTimerAt(int64_t when_ms, const TimerTask& handler);
    uint32_t CreateTimerAfter(int64_t delay_ms, const TimerTask& handler);
    uint32_t CreateTimerEvery(int64_t interval_ms, const TimerTask& handler);

    void CancelTimer(uint32_t timer_id);

    bool Start();
    void Stop();

    void AppendTimeWheel(uint32_t scales, uint32_t scale_unit_ms, const std::string& name = "");

private:
    void Run();

    TimeWheelPtr GetGreatestTimeWheel();
    TimeWheelPtr GetLeastTimeWheel();

private:
    std::mutex mutex_;
    std::future<void> future;

    bool stop_;

    std::unordered_set<uint32_t> cancel_timer_ids_;

    uint32_t timer_step_ms_;
    std::vector<TimeWheelPtr> time_wheels_;
};

static uint32_t s_inc_id = 1;

TimeWheelScheduler::TimeWheelScheduler(uint32_t timer_step_ms) :
    stop_(false),
    timer_step_ms_(timer_step_ms) {
}

TimeWheelScheduler::~TimeWheelScheduler() {
    Stop();
}

bool TimeWheelScheduler::Start()  {
    if (timer_step_ms_ < 50) {
        return false;
    }

    if (time_wheels_.empty()) {
        return false;
    }

    future = std::async(std::launch::async, &TimeWheelScheduler::Run, this);

    return true;
}

void TimeWheelScheduler::Run() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timer_step_ms_));
        std::lock_guard<std::mutex> lock(mutex_);
        if (stop_) {
            break;
        }

        TimeWheelPtr least_time_wheel = GetLeastTimeWheel();
        least_time_wheel->Increase();
        std::list<TimerPtr> slot = least_time_wheel->GetAndClearCurrentSlot();
        for (const TimerPtr& timer : slot) {
            auto it = cancel_timer_ids_.find(timer->id());
            if (it != cancel_timer_ids_.end()) {
                cancel_timer_ids_.erase(it);
                continue;
            }

            timer->Run();
            if (timer->repeated()) {
                timer->UpdateWhenTime();
                GetGreatestTimeWheel()->AddTimer(timer);
            }
        }
    }
}

void TimeWheelScheduler::Stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    if (future.valid() == true) {
        future.wait();
    }
}

TimeWheelPtr TimeWheelScheduler::GetGreatestTimeWheel() {
    if (time_wheels_.empty()) {
        return TimeWheelPtr();
    }

    return time_wheels_.front();
}

TimeWheelPtr TimeWheelScheduler::GetLeastTimeWheel()  {
    if (time_wheels_.empty()) {
        return TimeWheelPtr();
    }

    return time_wheels_.back();
}

void TimeWheelScheduler::AppendTimeWheel(uint32_t scales, uint32_t scale_unit_ms, const std::string& name) {
    TimeWheelPtr time_wheel = std::make_shared<TimeWheel>(scales, scale_unit_ms, name);
    print(name);
    if (time_wheels_.empty()) {
        time_wheels_.push_back(time_wheel);
        return;
    }

    TimeWheelPtr greater_time_wheel = time_wheels_.back();
    // print(greater_time_wheel->)
    greater_time_wheel->set_less_level_tw(time_wheel.get());
    time_wheel->set_greater_level_tw(greater_time_wheel.get());
    time_wheels_.push_back(time_wheel);
}

uint32_t TimeWheelScheduler::CreateTimerAt(int64_t when_ms, const TimerTask& handler) {
    if (time_wheels_.empty()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    ++s_inc_id;
    GetGreatestTimeWheel()->AddTimer(std::make_shared<Timer>(s_inc_id, when_ms, 0, handler));

    return s_inc_id;
}

uint32_t TimeWheelScheduler::CreateTimerAfter(int64_t delay_ms, const TimerTask& handler) {
    int64_t when = GetNowTimestamp() + delay_ms;
    return CreateTimerAt(when, handler);
}

uint32_t TimeWheelScheduler::CreateTimerEvery(int64_t interval_ms, const TimerTask& handler) {
    if (time_wheels_.empty()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    ++s_inc_id;
    int64_t when = GetNowTimestamp() + interval_ms;
    GetGreatestTimeWheel()->AddTimer(std::make_shared<Timer>(s_inc_id, when, interval_ms, handler));

    return s_inc_id;
}

void TimeWheelScheduler::CancelTimer(uint32_t timer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    cancel_timer_ids_.insert(timer_id);
}

int test_timer() {
    // Four level time wheels: Hour, Minute, Secon, Millisecond
    int timer_step_ms = 50;
    TimeWheelScheduler tws(timer_step_ms);
    // Hour time wheel. 24 scales, 1 scale = 60 * 60 * 1000ms.
    tws.AppendTimeWheel(24, 60 * 60 * 1000, "HourTimeWheel");
    // Minute time wheel. 60 scales, 1 scale = 60 * 1000ms
    tws.AppendTimeWheel(60, 60 * 1000, "MinuteTimeWheel");
    // Second time wheel. 60 scales, 1 scale = 1000ms.
    tws.AppendTimeWheel(60, 1000, "SecondTimeWheel");
    // Millisecond time wheel. 1000/timer_step_ms scales, 1 scale = timer_step_ms.
    tws.AppendTimeWheel(1000 / timer_step_ms, timer_step_ms, "MillisecondTimeWheel");

    print("tws start!");
    tws.Start();

    print(getCurrentTime(), "tws 10s!");
    tws.CreateTimerAt(GetNowTimestamp() + 10000, []() {
        print(getCurrentTime(), "10s");
    });

    print(getCurrentTime(), "tws 0.5s!");
    tws.CreateTimerAt(GetNowTimestamp() + 500, []() {
        print(getCurrentTime(), "0.5s");
    });

    print(getCurrentTime(), "tws 1s!");
    tws.CreateTimerAt(GetNowTimestamp() + 1000, []() {
        print(getCurrentTime(), "1s");
    });

    // print(getCurrentTime(), "tws 0.5s!");
    // tws.CreateTimerAfter(500, []() {
    //     print(getCurrentTime(), "0.5s");
    // });

    // print(getCurrentTime(), "tws 5s!");
    // auto timer_id = tws.CreateTimerEvery(5000, []() {
    //     print(getCurrentTime(), "every 5s");
    // });

    // print(getCurrentTime(), "tws 30s!");
    // tws.CreateTimerEvery(30 * 1000, []() {
    //     print(getCurrentTime(), "every 30s");
    // });

    // print(getCurrentTime(), "tws cancel 5s!");
    // tws.CancelTimer(timer_id);

    std::this_thread::sleep_for(std::chrono::minutes(20));
    print(getCurrentTime(), "tws stop!");
    tws.Stop();
    

    return 0;
}