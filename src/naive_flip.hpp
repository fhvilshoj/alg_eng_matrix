#ifndef ALG_NAIVE_FLIP
#define ALG_NAIVE_FLIP

#include "helper.hpp"

namespace matmul {

    namespace naive_flip {
        namespace _impl {
            void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest){
                for (unsigned i = 0u; i < m; i++){
                    for(unsigned j = 0u; j < p; j++){
                        for(unsigned k = 0u; k < n; k++){
                            dest[i][j] += A[i][k] * B[j][k];
                        }
                    }
                }
            }
        }

        /**
         * Transposes B to obtain less cache faults
         * @param A
         * @param B
         * @param m
         * @param n
         * @param p
         */
        bool build(int **&A, int **&B, unsigned const m, unsigned const n, unsigned const p){
            int **newB;
            helper::matrix::initialize_matrix(newB, p, n);

            for(unsigned i = 0; i < p; i ++){
                for(unsigned j = 0; j < n; j++){
                    newB[i][j] = B[j][i];
                }
            }
            int **oldB = B;
            B = newB;
            helper::matrix::destroy_matrix(oldB);
            return true;
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
