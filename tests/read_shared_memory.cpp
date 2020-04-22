#include <iostream>
#include <cassert>
#include "../capio.hpp"

struct elem {
    int i;
    double d;
    elem() {
        i = 0;
        d = 0;
    }
};

/*
 * single consumer that read a sequence of integers and a sequence of structs through the capio proxy
 * to use in combination with a producer
 */

int const num_writes = 45;

int main(int argc, char** argv) {
    capio_proxy<int> proxy("outputfile7", 1, 10);
    std::cout << "after constuctor\n";
    int i, j = 0;
    while (proxy.read(& i)) {
        std::cout << "read\n";
        assert(i == j);
        std::cout << i << "\n";
        ++j;
    }
    capio_proxy<struct elem> proxy2("outputfile8", 1, 12);
    std::cout << "after constuctor\n";
    struct elem e;
    j = 0;
    while (proxy2.read(&e)) {
        std::cout << "read\n";
        assert(e.i == j);
        assert(e.d == (double) j / num_writes);
        std::cout << e.i << " " << e.d << "\n";
        ++j;
    }
}

