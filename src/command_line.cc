#include "command_line.hh"
#include "command.hh"
#include "command_buffer.hh"
#include <iostream>
#include <vector>
#include <list>
#include <future>

class CommendLine::CommendLineImpl
{
public:
    CommendLineImpl() : 
        iter(observer_list_.begin()) {}
    
    ~CommendLineImpl() {
        if (future.valid()) {
            future.wait();
        }
    }

    void start() {
        future = std::async(std::launch::async, &CommendLineImpl::read, this);
    }

    void read() {
        std::cout << "Please Enter: " << std::endl;
        while(std::getline(std::cin, cmd))
        {
            if (!checkCmd(cmd)) 
            {
                std::cout << std::endl << "Enter context is error, please enter again: " << std::endl;
                continue;
            }
            auto ptr = std::make_unique<Command>(cmd);
            notify(std::move(ptr));
            std::cout << std::endl << "Please Enter: " << std::endl;
        }
    }

    void unit_test() {  // unit_test
        for (auto& str : work_queue_) {
            std::cout << str << std::endl;
        }
        work_queue_.clear();
    }

    void attach(CommandBuffer* observer) {
        observer_list_.push_back(observer);
    }

private:
    bool checkCmd(const std::string& cmd) {
        for (auto& ch : cmd)
        {
            if (ch == 32)
            {
                continue;
            }
            if (isCharacter(ch))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    bool isCharacter(const char& c)
    {
        if (((c >= 65) && (c <= 90)) || ((c >= 97) && (c <= 122))) 
        {
            return true;
        }
        return false;
    }

    void notify(std::unique_ptr<Command> pcmd) 
    {
        if (observer_list_.empty()) {
            return;
        }

        if (iter == observer_list_.end()) {
            iter = observer_list_.begin();
        }

        (*iter)->pushCmd(std::move(pcmd));
        ++iter;
    }

private:
    std::string cmd;
    std::vector<std::string> work_queue_;
    std::list<CommandBuffer*> observer_list_;
    std::list<CommandBuffer*>::iterator iter;
    std::future<void> future;
};

CommendLine::CommendLine() :
    pimpl(std::make_unique<CommendLineImpl>()) {}

CommendLine::~CommendLine() {}

void CommendLine::read() {
    pimpl->read();
}

void CommendLine::attach(CommandBuffer* observer)
{
    pimpl->attach(observer);
}

void CommendLine::Start() {
    pimpl->start();
}