#ifndef ALG_OBLIVIOUS_S_FLIP
#define ALG_OBLIVIOUS_S_FLIP

#include "helper.hpp"

namespace matmul {

    namespace oblivious_s_flip {

        namespace _impl {

            void multiply_naive(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest, unsigned const doffset, unsigned const aoffset, unsigned const boffset){
                for(unsigned i = 0u; i < m; i++){
                    for(unsigned j = 0u; j < p; j++){
                        for(unsigned k = 0u; k < n; k++){
                            dest[i][doffset + j] += A[i][aoffset + k] * B[j][boffset + k];
                        }
                    }
                }
            }

            void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest, unsigned const option, unsigned const doffset = 0, unsigned const aoffset = 0, unsigned const boffset = 0){
                const unsigned max = std::max(m, std::max(n, p));
                if(max < option){
                    multiply_naive(A, B, m, n, p, dest, doffset, aoffset, boffset);
                } else if(m >= std::max(n, p)){
                    // case 1
                    const unsigned split = m / 2;
                    const unsigned rest = m - split;
                    multiply(A, B, split, n, p, dest, option, doffset, aoffset, boffset);
                    multiply(A + split, B, rest, n, p, dest + split, option, doffset, aoffset, boffset);
                } else if(n >= std::max(m, p)){
                    //case 2
                    const unsigned split = n / 2;
                    const unsigned rest = n - split;
                    multiply(A, B, m, split, p, dest, option, doffset, aoffset, boffset);
                    multiply(A, B, m, rest, p, dest, option, doffset, aoffset + split, boffset + split);
                } else {
                    //case 3
                    const unsigned split = p / 2;
                    const unsigned rest = p - split;
                    multiply(A, B, m, n, split, dest, option, doffset, aoffset, boffset);
                    multiply(A, B + split, m, n, rest, dest, option, doffset + split, aoffset, boffset);
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
            _impl::multiply(A, B, m, n, p, destination, option);
        }
    }
}

#endif
