#include "command.hh"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

class Command::CommandImpl 
{
public:
    CommandImpl(const std::string& cmd) {
        std::istringstream istr(cmd);
        std::string tmp_str;
        while (istr >> tmp_str)
        {
            work_queue.emplace_back(tmp_str);
        }
    }

    uint32_t getCmdLength() 
    {
        return work_queue.size();
    }

    const std::string& operator[](uint32_t index) const
    {
        return work_queue.at(index);
    }
private:
    std::vector<std::string> work_queue;
};

Command::Command(const std::string& cmd) :
    pimpl(std::make_unique<CommandImpl>(cmd)) {
}

Command::~Command() {
}

uint32_t Command::getCmdLength()
{
    return pimpl->getCmdLength();
}

const std::string Command::operator[](uint32_t index) const
{
    return pimpl->operator[](index);
}