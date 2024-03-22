#include "command_buffer.hh"
#include "command_line.hh"
#include <deque>
#include <condition_variable>
#include <mutex>
// 条件变量的用法，以及条件变量、锁、信号量的区别

class CommandBuffer::CommandBufferImpl
{
public:
    Command&& getCmd()  // 需要测试
    {
        std::unique_lock<std::mutex> lck(mtx);
        if (cmd_queue.empty()) {
            cv.wait(lck, [this](){return !cmd_queue.empty();});
        }
        auto ret = std::move(cmd_queue.back());
        cmd_queue.pop_back();
        return std::move(*ret);
    }

    void pushCmd(std::unique_ptr<Command> cmd)
    {
        std::unique_lock<std::mutex> lck(mtx);
        if (cmd_queue.empty()) 
        {
            cmd_queue.emplace_back(std::move(cmd));
            cv.notify_one();
        } 
        else 
        {
            cmd_queue.emplace_back(std::move(cmd));
        }
    }

private:
    std::deque<std::unique_ptr<Command>> cmd_queue;
    std::condition_variable cv;
    std::mutex mtx;
};

CommandBuffer::CommandBuffer() :
    pimpl(std::make_unique<CommandBufferImpl>()) {
}

CommandBuffer::~CommandBuffer() {}

Command&& CommandBuffer::getCmd()
{
    return pimpl->getCmd();
}

void CommandBuffer::pushCmd(std::unique_ptr<Command> pcmd) 
{
    pimpl->pushCmd(std::move(pcmd));
}

void CommandBuffer::record(CommendLine& object) {
    object.attach(this);
}