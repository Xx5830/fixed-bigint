#include <fbi/fixed_bigint.hpp>
#include <iostream>

int main() {
    fbi::fixed_bigint<2025> a("999999999999999999999999999999");
    fbi::fixed_bigint<2025> b("2");

    std::cout << "A = " << a << "\n";
    std::cout << "B = " << b << "\n";
    std::cout << "A * B = " << (a * b) << "\n";

    return 0;
}