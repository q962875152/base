#include <type_traits>
#include <iostream>
#include "../MWUtils/singleton.h"

// ReturnType (Class::*ptrName)(parameters) = nullptr;

class A : public Singleton<A> {
    friend class Singleton<A>;
public:
    void test() {}
private:
    A() = default;
};

// class B
// {
// public:
//     virtual ~B() {std::cout << "11111111" << std::endl;}
// };

// class C : public B
// {
// };

// int main() {
    // if (std::is_member_function_pointer<decltype(A::test)>::value) {
    //     std::cout << "成员函数指针" << std::endl;
    // } else {
    //     std::cout << "非成员函数指针" << std::endl;
    // }
    // std::cout << &Singleton<A>::Instance() << std::endl;
    // Singleton<A>::Instance();
    // B *b = new C();
    // delete b;
// }