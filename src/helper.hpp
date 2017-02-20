
namespace helper {

    void initialize_matrix(int **&M, unsigned const m, unsigned const n){
        M = (int **) std::calloc(m, sizeof(int*));
        for(unsigned i = 0; i < m; i++){
            M[i] = (int*) std::calloc(n, sizeof(int));
        }
    }

    /***
     * Free m rows in M and lastly frees M it selves.
     * @param M
     * @param m
     */
    void destroy_matrix(int **&M, unsigned m){
        for(unsigned i = 0; i < m; i++){
            std::free(M[i]);
        }
        std::free(M);
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


}
