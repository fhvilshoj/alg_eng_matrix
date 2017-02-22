#include<cstdlib>
#include<iostream>
#include<string>

#include "../src/helper.hpp"

struct layout_profile
{
    std::string extension;
    typedef void (*build_delegate)(int const *inorder, unsigned size, int *&layout, unsigned &layout_size);
    typedef void (*destroy_delegate)(int *layout, unsigned layout_size);
    build_delegate build;
    destroy_delegate destroy;
};

unsigned parse_uint(char const *str)
{
    if (!str)
        return -1u;
    unsigned result = 0u;
    for (unsigned digit; *str; ++str)
    {
        if (*str < '0' || *str > '9')
            return -1u;
        digit = *str - '0';
        if (result > 429496729u
            || (result == 429496729u && digit >= 6u))
            return -1u;
        result = result * 10u + digit;
    }
    return result;
}

void print_usage()
{
    fputs
        (
            "Usage: data_gen <m> <n> <p> <file>\n"
                "<m>       Number of rows for A.\n"
                "<n>       Number of cols for A.\n"
                "          and number of rows for B.\n"
                "<p>       Number of rows for B\n"
                "<file>    The name of the output file.\n"
                "\n"
                "The generated data will be two csv's separated by a blank line",
            stderr
        );
}

int main(int argc, char **argv)
{
    unsigned m, n, p;

    if (argc != 5
        || (m = parse_uint(argv[1])) < 1u
        || (n = parse_uint(argv[2])) < 1u
        || (p = parse_uint(argv[3])) < 1u)
    {
        print_usage();
        return 1;
    }
    /* Output the query information. */
    {
        FILE *f = std::fopen(argv[4], "w");
        std::fprintf(f, "%u %d %d\n", m, n, p);
        std::fclose(f);
    }
    /* Generate the layouts. */

    int **layoutA;
    int **layoutB;

    helper::matrix::initialize_matrix(layoutA, m, n);
    helper::matrix::initialize_matrix(layoutB, n, p);
    helper::matrix::fill_matrix(layoutA, m, n);
    helper::matrix::fill_matrix(layoutB, n, p);

    FILE *f = std::fopen(argv[4], "w");
    helper::file_handler::layout_file::save(f, layoutA, layoutB, m, n, p);
    fclose(f);

    helper::matrix::destroy_matrix(layoutA);
    helper::matrix::destroy_matrix(layoutB);

    return 0;
}
