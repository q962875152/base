#include <cstdint>
#include <cstddef>

class IO 
{
 public:
    virtual ~IO() = default;

    virtual int32_t read(void* buf, size_t count);
    virtual int32_t write(const void* buf, size_t count);
};