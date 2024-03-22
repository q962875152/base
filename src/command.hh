#pragma once

#include <memory>
#include <string>

class Command 
{
public:
    Command(const std::string& cmd);
    ~Command();
    uint32_t getCmdLength();
    const std::string operator[](uint32_t index) const;
private:
    class CommandImpl;
    std::unique_ptr<CommandImpl> pimpl;
};