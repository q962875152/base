#include "threadpool.hh"
#include "log.hh"
#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include  <functional>

class Task {
public:
    template<typename F, typename... Args>
    Task(F&& f, Args&&... args) : func(std::bind(f, args...)) {
    }

    void run() {
        func();
    }
private:
    std::function<void()> func;
};

class TaskQueue {
public:
    void push(std::unique_ptr<Task> ptr) {
        std::unique_lock<std::mutex> lck(mtx);
        task_queue_.push_back(std::move(ptr));
        cv.notify_one();
    }

    std::unique_ptr<Task> get_and_pop() {
        std::unique_lock<std::mutex> lck(mtx);
        if (task_queue_.empty()) {
            cv.wait(lck, [&](){return !task_queue_.empty();});
        }
        auto ptr = std::move(task_queue_.front());
        task_queue_.pop_front();
        return std::move(ptr);
    }

    bool empty() {
        std::unique_lock<std::mutex> lck(mtx);
        return task_queue_.empty();
    }

    uint32_t size() {
        std::unique_lock<std::mutex> lck(mtx);
        return task_queue_.size();
    }
private:
    std::deque<std::unique_ptr<Task>> task_queue_;
    std::condition_variable cv;
    std::mutex mtx;
};

class ThreadPool::ThreadPoolImpl {
public:
    ThreadPoolImpl() : 
        handler_queue_(2) ,
        max_hander_nums_(std::thread::hardware_concurrency()),
        isStop(false) {
    }

    ~ThreadPoolImpl() {
        for (auto i = 0; i < handler_queue_.size(); ++i) {
            if (handler_queue_[i].joinable()) {
                handler_queue_[i].join();
            }
        }
    }

    void Start() {
        for (auto i = 0; i < 2; ++i) {
            handler_queue_[i] = std::thread(&ThreadPoolImpl::ThreadRun, this);
        }
    }

    void runTask(std::unique_ptr<Task> task) {
        if (!task_queue_.empty()) {
            if (handler_queue_.size() <= max_hander_nums_) {
                handler_queue_.emplace_back(std::thread(&ThreadPoolImpl::ThreadRun, this));
            }
        }

        task_queue_.push(std::move(task));
    }

    void Stop() {
        isStop.store(true);
    }

private:
    void ThreadRun() {
        LOG("This thread has been created!");
        while (1) {
            if (isStop) {
                return;
            }
            auto task = task_queue_.get_and_pop();
            task->run();
        }
    }

private:
    std::deque<std::thread> handler_queue_;
    TaskQueue task_queue_;
    uint32_t max_hander_nums_;
    std::atomic_bool isStop;
};

ThreadPool::ThreadPool() :
    pimpl_(std::make_unique<ThreadPoolImpl>()) {
}

ThreadPool::~ThreadPool() = default;

template<typename F, typename... Args>
void ThreadPool::push(F&& f, Args&&... args) {
    pimpl_->runTask(std::make_unique<Task>(f, args...));
}

void ThreadPool::push(std::unique_ptr<Task> task) {
    pimpl_->runTask(std::move(task));
}

void ThreadPool::Start() {
    pimpl_->Start();
}

void ThreadPool::Stop() {
    pimpl_->Stop();
}

void ThreadPoolTest() {
//     ThreadPoolProcess.Start();
}