#ifndef ALG_OBLIVIOUS
#define ALG_OBLIVIOUS

#include "helper.hpp"

namespace matmul {

    namespace oblivious {

        namespace _impl {
            void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest, unsigned const doffset = 0, unsigned const aoffset = 0, unsigned const boffset = 0){
                if(m == 0 || n == 0 || p == 0){
                    return;
                }
//                std::cout << "A topleft: " << A[0][0] << " B top left: " << B[0][0] << " m " << m << " n " << n << " p " << p << " doff: " << doffset << " aoff " << aoffset << " boff " << boffset << std::endl;
                if(m == 1 && n == 1 && p == 1){
                    dest[0][doffset + 0] += A[0][aoffset + 0] * B[0][boffset + 0];
                }
                else if(m >= (n >= p ? n : p)){
                    // case 1
                    const unsigned split =  m >> 1; // divide by 2
                    const unsigned rest = m - split;
                    multiply(A, B, split, n, p, dest, doffset, aoffset, boffset);
                    multiply(A + split, B, rest, n, p, dest + split, doffset, aoffset, boffset);
                } else if(n >= (m >= p ? m : p)){
                    //case 2
                    const unsigned split = n >> 1; // divide by 2
                    const unsigned rest = n - split;
                    multiply(A, B, m, split, p, dest, doffset, aoffset, boffset);
                    multiply(A, B + split, m, rest, p, dest, doffset, aoffset + split, boffset);

                } else {
                    //case 3
                    const unsigned split = p >> 1; // divide by 2
                    const unsigned rest = p - split;
                    multiply(A, B, m, n, split, dest, doffset, aoffset, boffset);
                    multiply(A, B, m, n, rest, dest, doffset + split, aoffset, boffset + split);
                }
            }
        }

        /**
         * Empty
         * @param A
         * @param B
         * @param m
         * @param n
         * @param p
         */
        bool build(int **&A, int **&B, unsigned const m, unsigned const n, unsigned const p){
            return false;
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

#endif
