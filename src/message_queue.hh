#ifndef MESSAGE_QUEUE_HH_
#define MESSAGE_QUEUE_HH_

#include "log.hh"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

class Message {
public:
    virtual ~Message() {};

    virtual void test_print() = 0;
};

template <typename T>
class Message_Queue {
public:
    void push(std::unique_ptr<T> message) {
        std::lock_guard<std::mutex> lck(mtx);
        queue.emplace_back(std::move(message));
        cv.notify_one();
    }

    std::unique_ptr<T> wait() {
        std::unique_lock<std::mutex> lck(mtx);
        if (queue.empty()) {
            cv.wait(lck, [&]{return !queue.empty();});
        }
        auto ret = std::move(queue.front());
        queue.pop_back();
        return ret;
    }

    std::unique_ptr<T> poll() {
        std::lock_guard<std::mutex> lck(mtx);
        if (queue.empty()) {
            return nullptr;
        }
        auto ret = std::move(queue.front());
        queue.pop();
        return ret;
    }

private:
    std::vector<std::unique_ptr<T>> queue;
    std::mutex mtx;
    std::condition_variable cv;
};


extern Message_Queue<Message> MQ;

class testmessage : public Message {
public:
    testmessage(const std::string& temp) :
        str(temp) {}
    virtual void test_print() override {
        LOG(str);
    }
    virtual ~testmessage() {};
    std::string str;
};

inline void mq_test1() {
    // LOG();
    MQ.push(std::make_unique<testmessage>("1111111111"));
    MQ.push(std::make_unique<testmessage>("222222222"));
    MQ.push(std::make_unique<testmessage>("33333333333"));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    MQ.push(std::make_unique<testmessage>("444444444444"));
}

inline void mq_test2() {
    MQ.wait()->test_print();
    MQ.wait()->test_print();
    MQ.wait()->test_print();
    MQ.wait()->test_print();
}

inline void MessageQueueTest() {
    std::thread t1(mq_test1);
    std::thread t2(mq_test2);

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
}

#endif