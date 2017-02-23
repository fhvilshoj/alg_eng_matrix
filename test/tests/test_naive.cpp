#include <iostream>

#include "../lib/catch.hpp"
#include "../../src/helper.hpp"
#include "../../src/naive.hpp"
#include "../../src/oblivious.hpp"


TEST_CASE("First simple testcase", "[Naive, empty]"){
    int **resA;
    int **resB;
    int **resBs;
    int **A;
    int **B;
    int const m = 9;
    int const n = 11;
    int const p = 5;

    helper::matrix::initialize_matrix(A, m, n);
    helper::matrix::initialize_matrix(B, n, p);

    helper::matrix::initialize_matrix(resA, m, p);
    helper::matrix::initialize_matrix(resB, m, p);
    helper::matrix::initialize_matrix(resBs, m, p);

    unsigned a = 0;
    for (unsigned i = 0u; i < m; i++){
        for(unsigned j = 0u; j < n; j++){
            A[i][j] = a;
            a++;
        }
    }
    a = 1;
    for (unsigned i = 0u; i < n; i++){
        for(unsigned j = 0u; j < p; j++){
            B[i][j] = a;
            a++;
        }
    }
    matmul::naive::multiply((const int **) A, (const int **) B, m, n, p, resA, 0);
    matmul::oblivious::multiply((const int **) A, (const int **) B, m, n, p, resB, 0);
    matmul::oblivious::multiply((const int **) A, (const int **) B, m, n, p, resBs, 10);

    int result[m][p] = {
        { 1980, 2035, 2090, 2145, 2200},
        { 5126, 5302, 5478, 5654, 5830},
        { 8272, 8569, 8866, 9163, 9460},
        {11418,11836,12254,12672,13090},
        {14564,15103,15642,16181,16720},
        {17710,18370,19030,19690,20350},
        {20856,21637,22418,23199,23980},
        {24002,24904,25806,26708,27610},
        {27148,28171,29194,30217,31240}
    };

    for (unsigned i = 0; i < m; ++i) {
        for(unsigned j = 0; j < p; j++){
            REQUIRE(result[i][j] == resA[i][j]);
            REQUIRE(resA[i][j] == resB[i][j]);
            REQUIRE(resB[i][j] == resBs[i][j]);
        }
    }

    helper::matrix::destroy_matrix(A);
    helper::matrix::destroy_matrix(B);
    helper::matrix::destroy_matrix(resA);
    helper::matrix::destroy_matrix(resB);
    helper::matrix::destroy_matrix(resBs);
}
