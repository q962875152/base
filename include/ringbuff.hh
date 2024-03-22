#ifndef RINGBUFF_HH_
#define RINGBUFF_HH_

#include "cstring"
#include <cstdint>
#include <ostream>
#include "iostream"
#include "atomic"

// 需要使用原子变量实现线程安全  结构体中的软件

class Ringbuff {  // 一个线程读，一个线程写不会报错
public:
    Ringbuff(int32_t size) :
        buf_(nullptr),
        size_(size),
        mask_(size - 1),
        head_(0),
        tail_(0) {
        if (size_ != 0) {
            // 大小向上取整，保证ringbuff是2的幂
            if (size_ & (size_ - 1)) {
                int position = 0;
                for (int i = size_; i != 0; i >>= 1) {
                    ++position;
                }
                size_ = 1 << position;
                mask_ = size_ - 1;
            }

            buf_ = static_cast<int32_t*>(new int32_t[size_]);
            std::memset(buf_, 0, sizeof(int32_t) * size_);
        } else {
            std::cout << "buffer size is zero" << std::endl;
        }
    }

    void push(int cmd) {
        if (tail_ ==  (head_ ^ size_)) {  // 为啥，需要确定
            // 命令发送速度过快处理
            return;
        }
        buf_[tail_ & mask_] = cmd;  // 按位与运算
        ++tail_;
    }

    int pop() {
        if (tail_ == head_) {
            std::cout << "no new commend" << std::endl;
            return 0;
        }

        int cmd = buf_[head_ & mask_];
        ++head_;
        return cmd;
    }

    void printData() {
        std::cout << size_ << std::endl;
        for (int i = 0; i < size_; ++i) {
            if ((i >= (head_ % size_)) && (i < (tail_ % size_)))
            std::cout << buf_[i] << std::endl;
        }
    }

    ~Ringbuff() {
        delete[] buf_;
    }
private:
    int32_t* buf_;
    uint32_t size_;
    uint32_t mask_;
    std::atomic_uint64_t head_;
    std::atomic_uint64_t tail_;
};

#endif