#include "iostream"

template <typename T>
int test(T a) {
    std::cout << "test" << std::endl;
    return 0;
}

// template <typename T>
// class Test {
// public:
//     int test();
// private:
//     T a;
// };

// template <typename T>
// int Test<T>::test() {
//     std::cout << "test" << std::endl;
//     return 0;
// }