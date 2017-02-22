#ifndef ALG_OBLIVIOUS_S
#define ALG_OBLIVIOUS_S

#include "helper.hpp"

namespace matmul {

    namespace oblivious_s {

        namespace _impl {

            void multiply_naive(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest, unsigned const doffset, unsigned const aoffset, unsigned const boffset){
                for(unsigned i = 0u; i < m; i++){
                    for(unsigned j = 0u; j < n; j++){
                        for(unsigned k = 0u; k < p; k++){
                            dest[i][doffset + j] += A[i][aoffset + k] * B[k][boffset + j];
                        }
                    }
                }
            }

            void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest, unsigned const doffset = 0, unsigned const aoffset = 0, unsigned const boffset = 0){
                if(m == 0 || n == 0 || p == 0){
                    return;
                }
//                std::cout << "A topleft: " << A[0][0] << " B top left: " << B[0][0] << " m " << m << " n " << n << " p " << p << " doff: " << doffset << " aoff " << aoffset << " boff " << boffset << std::endl;
                const unsigned max = std::max(m, std::max(n, p));
                if(max < 10){
                    multiply_naive(A, B, m, n, p, dest, doffset, aoffset, boffset);
                }
                else if(m >= std::max(n, p)){
                    // case 1
                    const unsigned split = m / 2;
                    const unsigned rest = m - split;
                    multiply(A, B, split, n, p, dest, doffset, aoffset, boffset);
                    multiply(A + split, B, rest, n, p, dest + split, doffset, aoffset, boffset);
                } else if(n >= std::max(m, p)){
                    //case 2
                    const unsigned split = n / 2;
                    const unsigned rest = n - split;
                    multiply(A, B, m, split, p, dest, doffset, aoffset, boffset);
                    multiply(A, B + split, m, rest, p, dest, doffset, aoffset + split, boffset);

                } else {
                    //case 3
                    const unsigned split = p / 2;
                    const unsigned rest = p - split;
                    multiply(A, B, m, n, split, dest, doffset, aoffset, boffset);
                    multiply(A, B, m, n, rest, dest, doffset + split, aoffset, boffset + split);
                }
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
        void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **&destination){
            if(m == 0 || n == 0 || p == 0){
                destination = nullptr;
                return;
            }

            destination =  (int **)std::calloc(m, sizeof(int*));
            for(unsigned i = 0u; i < m; i++){
                destination[i] = (int*)std::calloc(p, sizeof(int));
            }
            _impl::multiply(A, B, m, n, p, destination);
        }
    }
}

#endif
