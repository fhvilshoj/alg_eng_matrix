#ifndef ALG_HELPER_HPP
#define ALG_HELPER_HPP

#include <random>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

namespace helper {

    namespace matrix {
        void initialize_matrix(int **&M, unsigned const m, unsigned const n){
            int *tmp = (int *) std::calloc(m * n, sizeof(int));
            M = (int **) std::calloc(m, sizeof(int*));
            for(unsigned i = 0; i < m; i++){
                M[i] = tmp;
                tmp += n;
            }
        }

        /***
         * Free m rows in M and lastly frees M it selves.
         * @param M
         * @param m
         */
        void destroy_matrix(int **&M){
            if(M){
                if(M[0]){
                    std::free(M[0]);
                }
                std::free(M);
            }
            M = nullptr;
        }

        void print_matrix(int **M, unsigned const m, unsigned const n){
            std::cout << "---------------" << std::endl;
            for(unsigned i = 0u; i < m; i++){
                for(unsigned j = 0u; j < n; j++){
                    std::cout << M[i][j] << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "---------------" << std::endl;
        }


        int getRandom(const int min=-1000, const int max=1000) {
            return min + (rand() % (int)(max - min + 1));
        }

        void fill_matrix(int **M, unsigned const m, unsigned const n, const unsigned seed=1u){
            if(seed > 0u){
                srand(seed);
            } else {
                srand(time(NULL));
            }
            for(unsigned i = 0; i < m; i++){
                for(unsigned j = 0; j < n; j++){
                    M[i][j] = getRandom();
                }
            }
        }
    }

    namespace file_handler {
        struct layout_file
        {
            bool valid;
            int **layoutA;
            int **layoutB;
            unsigned layout_m;
            unsigned layout_n;
            unsigned layout_p;
            unsigned layout_capacity;
            layout_file(FILE *f = nullptr)
                : valid(false), layoutA(nullptr), layoutB(nullptr), layout_n(0u), layout_m(0u), layout_p(0u), layout_capacity(0u)
            {
                replace(f);
            }
            void dispose()
            {
                valid = false;
                matrix::destroy_matrix(layoutA);
                matrix::destroy_matrix(layoutB);
                layout_m = 0u;
                layout_n = 0u;
                layout_p = 0u;
            }
            void replace(FILE *f)
            {
                if (!f)
                {
                    dispose();
                    return;
                }
                unsigned m, n, p;
                if (std::fscanf(f, "%u %u %u", &m, &n, &p) != 3)
                {
                    dispose();
                    return;
                }
                if (!valid || m > layout_m || n > layout_n || p > layout_p)
                {
                    dispose();
                    matrix::initialize_matrix(layoutA, m, n);
                    matrix::initialize_matrix(layoutB, n, p);
                }
                layout_m = m;
                layout_n = n;
                layout_p = p;

                for (unsigned i = 0; i < m; i++){
                    for(unsigned j = 0; j < n; j++){
                        if (std::fscanf(f, "%d", layoutA[i] + j) != 1)
                        {
                            dispose();
                            return;
                        }
                    }
                }
                for (unsigned i = 0; i < n; i++){
                    for(unsigned j = 0; j < p; j++){
                        if (std::fscanf(f, "%d", layoutB[i] + j) != 1)
                        {
                            dispose();
                            return;
                        }
                    }
                }
                valid = true;
            }
            static void save(FILE *f, int **layoutA, int **layoutB, unsigned m, unsigned n, unsigned p)
            {
                std::fprintf(f, "%u %u %u\n", m, n, p);
                for (unsigned i = 0; i < m; ++i) {
                    for (unsigned j = 0; j < n; ++j) {
                        std::fprintf(f, "%d\n", layoutA[i][j]);
                    }
                }
                for (unsigned i = 0; i < n; ++i) {
                    for (unsigned j = 0; j < p; ++j) {
                        std::fprintf(f, "%d\n", layoutB[i][j]);
                    }
                }
            }
            ~layout_file()
            {
                dispose();
            }
        };
    }

}

#endif
