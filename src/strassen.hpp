#ifndef ALG_NAIVE
#define ALG_NAIVE

#include "omp.h"

namespace matmul {

    namespace strassen {
        namespace _impl {
            void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest){

                /*     A11 A12
                 * A = A21 A22
                 *
                 *     B11 B12
                 * B = B21 B22
                 *
                 *     C11 C12
                 * C = C21 C22
                 *
                 * C11 = A11B11 + A12B21
                 * C12 = A11B12 + A12B22
                 * C21 = A21B11 + A22B21
                 * C22 = A21B12 + A22B22
                 *
                 * M1 = (A11 + A22) (B11 + B12)
                 * M2 = (A21 + A22) B11
                 * M3 = A11 (B12 - B22)
                 * M4 = A22 (B21 - B11)
                 * M5 = (A11 + A12) B22
                 * M6 = (A21 - A11) (B11 + B12)
                 * M7 = (A12 - A22) (B21 + B22)
                 *
                 * C11 = M1 + M4 - M5 + M7
                 * C12 = M3 + M5
                 * C21 = M2 + M4
                 * C22 = M1 = M2 + M3 + M6
                 *
                 * /

                //TODO

            }
        }

        /***
         * Multiplys matrix A of size (m x n) with matrix B of size (n x p)
         * And adds the result to destination of size (m x p)
         * @param A
         * @param B
         * @param m
         * @param n
         * @param p
         * @param destination
         */
        void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **&destination, unsigned const option){
            if(m == 0 || n == 0 || p == 0){
                destination = nullptr;
                return;
            }
            _impl::multiply(A, B, m, n, p, destination);
        }
    }

}

#endif //ALG_NAIVE
