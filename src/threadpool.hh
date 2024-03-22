#ifndef THREADPOOL_HH_
#define THREADPOOL_HH_

#include "../MWUtils/singleton.h"
#include <memory>

class Task;

class ThreadPool : public Singleton<ThreadPool> {
    friend Singleton<ThreadPool>;
public:
    ~ThreadPool();

    template<typename F, typename... Args>
    void push(F&& f, Args&&... args);

    void push(std::unique_ptr<Task> task);

    void Start();
    void Stop();
private:
    ThreadPool();
private:
    class ThreadPoolImpl;
    std::unique_ptr<ThreadPoolImpl> pimpl_;
};

#define  ThreadPoolProcess Singleton<ThreadPool>::Instance()

void ThreadPoolTest();

#endif