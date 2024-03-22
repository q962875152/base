#include "../MWUtils/singleton.h"
#include <memory>

class TimerImple;

class Timer : public Singleton<Timer>
{
    friend Singleton<Timer>;
public:
    Timer();
    ~Timer();
    template <typename F, typename... Args>
    void setTimerTask(F &&f, Args &&... args);
    void clearTimerTask();
private:
    std::unique_ptr<Timer> pimpl_;
};