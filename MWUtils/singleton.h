#ifndef SINGLETON_HH_
#define SINGLETON_HH_

template <typename T>
class Singleton
{
public:
    static T& Instance() {
        static T Instance;
        return Instance;
    }
protected:
    virtual ~Singleton() = default;
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

// class DesignPattern : public Singleton<DesignPattern>
// {
//     friend class Singleton<DesignPattern>;
// public:
//     ~DesignPattern() = default;
// private:
//     DesignPattern() {}
// };

#endif