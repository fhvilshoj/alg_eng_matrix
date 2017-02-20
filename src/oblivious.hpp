#ifndef ALG_OBLIVIOUS
#define ALG_OBLIVIOUS

namespace matmul {

    namespace oblivious {

        namespace _impl {
            void multiply(){

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
        void multiply(int **A, int **B, unsigned m, unsigned n, unsigned p, int **destination){
            //TODO
            _impl::multiply();
        }
    }

}

#endif
