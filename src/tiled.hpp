#ifndef ALG_TILED
#define ALG_TILED

#include <algorithm>
#include "omp.h"

namespace matmul {

    namespace tiled {
        namespace _impl {

            void ord_mult(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, unsigned const a_off, unsigned const b_off, unsigned const d_off, int **dest) {
                for (unsigned i = 0u; i < m; i++){
                    for(unsigned j = 0u; j < p; j++){
                        for(unsigned k = 0u; k < n; k++){
                            dest[i][j + d_off] += A[i][k + a_off] * B[k][j + b_off];
                        }
                    }
                }
            }
            void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest, unsigned const option) {
                //assumes squared matrices
                unsigned a_off = 0u, b_off = 0u, d_off = 0u;
                unsigned num_tiles = (m / option) + 1;
                unsigned int a_row_idx = 0u;
                for(unsigned i = 0u; i < num_tiles; i++){
                    unsigned tile_m = std::min(option, m - a_row_idx);
                    unsigned int b_col_idx = 0u;
                    for(unsigned j = 0u; j < num_tiles; j++){
                        unsigned tile_p = std::min(option, p - b_col_idx); // j * option == b_off
                        for(unsigned k = 0u; k < num_tiles; k ++){
                            unsigned tile_n = std::min(option, n - a_off); // k * option == a_off
                            ord_mult(A + a_row_idx, B + a_off, tile_m, tile_n, tile_p, a_off, b_off, d_off, dest + a_row_idx);
                            a_off += option;
                        }
                        b_off += option;
                        d_off += option;
                        b_col_idx += option;
                        a_off = 0;
                    }
                    b_off = 0;
                    d_off = 0;
                    a_row_idx += option;                    
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
        void
        multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **&destination,
                 unsigned const option) {
            if (m == 0 || n == 0 || p == 0) {
                destination = nullptr;
                return;
            }
            _impl::multiply(A, B, m, n, p, destination, option);
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
    }

}

#endif //ALG_NAIVE
