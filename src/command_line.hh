#include "../MWUtils/singleton.h"
#include <memory>

class CommandBuffer;

class CommendLine : public Singleton<CommendLine>
{
    friend class Singleton<CommendLine>;
public:
    void read();
    void Start();
    void attach(CommandBuffer* observer);
    ~CommendLine();
private:
    CommendLine();
private:
    class CommendLineImpl;
    std::unique_ptr<CommendLineImpl> pimpl;
};

#define  CommandLineProcess  Singleton<CommendLine>::Instance()