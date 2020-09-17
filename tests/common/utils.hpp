#include <iostream>
#include <assert.h>

void print_array(int data[], int array_length, int rank) {
    std::cout << "array_length: " << array_length << std::endl;
    for (int i = 0; i < array_length; ++i) {
        std::cout << "reader " << rank << " data: " << data[i] << std::endl;
    }
}

void compare_expected_actual(int* actual, int* expected, int array_length) {
    for (int i = 0; i < array_length; ++i) {
        assert(actual[i] == expected[i]);
    }
}

void compute_expected_result_gather(int* expected_result, int array_length, int num_prods, int start) {
    for (int i = 0, num = start; i < array_length; ++i) {
        expected_result[i] = num;
        ++num;
        if ((i + 1) % (array_length / num_prods) == 0) {
            ++start;
            num = start;
        }
    }
}