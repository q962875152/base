#include <iostream>

class A {
public:
    void print() {
        std::cout << test() << std::endl;
    }
private:
    int test() {
        auto f = [&]() {
            return a + b;
        };
        return f();
    }
private:
    int a = 5;
    int b = 10;
};

// int main() {
    // int e = 5;
    // int h = 10;
    // auto f = [x = e, h](int &a, int b) {  // 被传值捕获的值默认是const
    //     a = 6;
    //     int c = 0;
    //     c = 1;
    //     // e = 4;
    //     return a + b + x;
    // };

    // int d = 5;

    // std::cout << f(d, 7) << std::endl;
    // std::cout << d << std::endl;
    // std::cout << e << std::endl;

    // A j;
    // j.print();


//     return 0;
// };