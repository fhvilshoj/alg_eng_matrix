#ifndef ALG_NAIVE
#define ALG_NAIVE
namespace matmul {

    namespace naive {
        namespace _impl {
            void multiply(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **dest){
                for (unsigned i = 0u; i < m; i++){
                    for(unsigned j = 0u; j < p; j++){
                        for(unsigned k = 0u; k < n; k++){
                            dest[i][j] += A[i][k] * B[k][j];
                        }
                    }
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
            //TODO
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

#endif //ALG_NAIVE
