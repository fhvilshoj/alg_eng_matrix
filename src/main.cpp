#include "papi.h"
#include <iostream>

#include "helper.hpp"
#include "naive.hpp"
#include "oblivious.hpp"

int main(){

    int counters[1] = {PAPI_BR_MSP};

    int **A;
    int **B;
    int **dest;
    long long arr[1] = {0l};
    long long arrRead[1] = {0l};

    helper::matrix::initialize_matrix(A, 200, 200);
    helper::matrix::initialize_matrix(B, 200, 200);
    helper::matrix::fill_matrix(A, 200, 200);
    helper::matrix::fill_matrix(B, 200, 200);

    PAPI_start_counters(counters, 1);

    matmul::naive::multiply((const int **) A, (const int **) B, 200, 200, 200, dest, 0);

    PAPI_read_counters(arrRead, 1);
//    PAPI_stop_counters(arr, 1);
    std::cout << arrRead[0] << " " << arr[0] << std::endl;
//    PAPI_start_counters(counters, 1);

    matmul::oblivious::multiply((const int **) A, (const int **) B, 50, 50, 100, dest, 0);

    PAPI_read_counters(arrRead, 1);
    PAPI_stop_counters(arr, 1);

    std::cout << arrRead[0] << " " << arr[0] << std::endl;

    return 0;
}
