#include "command.hh"
#include <memory>

class Command;
class CommendLine;

class CommandBuffer
{
public:
    CommandBuffer();
    ~CommandBuffer();
    // CommandBuffer()

    Command&& getCmd();
    void pushCmd(std::unique_ptr<Command> cmd);
    void record(CommendLine& object);

private:
    class CommandBufferImpl;
    std::unique_ptr<CommandBufferImpl> pimpl;
};