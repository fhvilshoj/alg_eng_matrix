#include<cstdlib>
#include<iostream>
#include<string>
#include <cstring>

#include "../src/naive.hpp"
#include "../src/oblivious.hpp"
#include "../src/oblivious_s.hpp"
#include "../src/helper.hpp"

#ifdef __GNUC__
#include<x86intrin.h>

namespace clocking
{
    inline unsigned long long ticks()
    {
        return __rdtsc();
    }
}
#else
#ifdef __MSVC__
#include<intrin.h>
namespace clocking
{
    inline unsigned long long ticks()
    {
        return __rdtsc();
    }
}
#else
namespace clocking
{
    inline unsigned long long ticks()
    {
        return std::clock();
    }
}
#endif
#endif


struct argument
{
    char const *name;
    char const *abbreviation;
    char const *description;
    unsigned min_count, max_count;
    enum { boolean, string, number } type;
    std::vector<bool> bool_values;
    std::vector<std::string> string_values;
    std::vector<unsigned> number_values;
    bool is(char const *testee) const
    {
        return std::strcmp(testee, abbreviation) == 0
               || std::strcmp(testee, name) == 0;
    }
} arguments[] =
    {
        { "input", "i", "Sets the input query file. Multiple ones allowed.", 1u, 128u, argument::string },
        { "output", "o", "Sets the output file name. Defaults to 'result'. This argument is ignored if algorithm is set.", 1u, 2u, argument::string, { }, { "result" }, { } },
        { "refresh", "r", "Sets the number of extra iterations to run before measuring the time. Defaults to 0, at most 1.000.000.000.", 1u, 2u, argument::number, { }, { }, { 0u } },
        { "algorithm", "a", "Sets the only algorithm to run. This is for isolated tests.", 0u, 1u, argument::string, { }, { }, { } },
        { "iterations", "l", "Sets the number of iterations to run when measuring the time. Defaults to 1000, at most 1.000.000.000.", 1u, 2u, argument::number, { }, { }, { 1000u } },
    };

constexpr int ARG_INPUT = 0;
constexpr int ARG_OUTPUT = 1;
constexpr int ARG_REFRESH = 2;
constexpr int ARG_ALGORITHM = 3;
constexpr int ARG_ITERATIONS = 4;

std::vector<std::string> files;
std::string output;
unsigned refresh_count;
unsigned iteration_count;


struct algorithm_profile
{
    typedef void (*multiply_delegate)(int const **A, int const **B, unsigned const m, unsigned const n, unsigned const p, int **&destination);
    char const *name;
    multiply_delegate multiply;
} algorithms[] =
    {
        { "naive", matmul::naive::multiply},
        { "obl", matmul::oblivious::multiply},
        { "obl:s", matmul::oblivious_s::multiply}
    };

algorithm_profile const *chosen;

void print_usage();
bool parse_arguments(int argc, char **argv);
bool validate_arguments();
void run_isolated_test(std::string const &dataset);
void run_test(std::string const &dataset, FILE *f);
void call_gnuplot();

int main(int argc, char **argv){
    std::cout << "Lets benchmark!" << std::endl;

    if (argc < 2)
    {
        print_usage();
        return 1;
    }
    if (!parse_arguments(argc, argv))
    {
        return 2;
    }
    if (!validate_arguments())
    {
        return 2;
    }
    if (chosen)
    {
        for (auto const &file : files)
            run_isolated_test(file);
    }
    else
    {
        FILE *f = fopen((output + ".data").c_str(), "w");
        for (auto const &a : algorithms)
        {
            fputs(a.name, f);
            fputc(' ', f);
            printf("%s\t", a.name);
        }
        fputs("m n p\n", f);
        puts("dataset\n");
        for (auto const &file : files)
            run_test(file, f);
        fclose(f);
        call_gnuplot();
    }
    return 0;
}

void print_usage()
{
    fputs
        (
            "Usage: benchmark <arguments>\n\n"
                "When specifying arguments, use '-<name or abbreviation>:<value>'.\n"
                "For boolean arguments (switches), you can also use '-<name or abbreviation>' to specify true.\n"
                "Valid values for boolean arguments are true/t/on/yes/y and false/f/off/no/n.\n"
                "Argument name and value are case-sensitive.\n\n",
            stderr
        );
    for (auto const &a : arguments)
        fprintf(stderr, "(%s) %s\n    %s\n", a.abbreviation, a.name, a.description);
}


bool parse_uint(char const *str, unsigned &result)
{
    if (!str)
        return false;
    unsigned x = 0u;
    for (unsigned digit; *str; ++str)
    {
        if (*str < '0' || *str > '9')
            return false;
        digit = *str - '0';
        if (x > 429496729u
            || (x == 429496729u && digit >= 6u))
            return false;
        x = x * 10u + digit;
    }
    result = x;
    return true;
}

bool parse_bool(char const *str, bool &result)
{
    static char const * const bool_true[] =
        {
            "true", "t", "on", "yes", "y", nullptr
        };
    static char const * const bool_false[] =
        {
            "false", "f", "off", "no", "n", nullptr
        };
    for (char const * const *p = bool_true; *p; ++p)
        if (std::strcmp(*p, str) == 0)
        {
            result = true;
            return true;
        }
    for (char const * const *p = bool_false; *p; ++p)
        if (std::strcmp(*p, str) == 0)
        {
            result = false;
            return true;
        }
    return false;
}

bool parse_arguments(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return false;
        }
        ++argv[i];
        char *value = std::strstr(argv[i], ":");
        /* switch */
        if (!value)
        {
            for (auto &a : arguments)
                if (a.type == argument::boolean && a.is(argv[i]))
                    a.bool_values.push_back(true);
        }
        else
        {
            *value++ = '\0';
            bool found = false;
            for (auto &a : arguments)
            {
                if (a.is(argv[i]))
                {
                    found = true;
                    bool bv;
                    unsigned nv;
                    switch (a.type)
                    {
                        case argument::boolean:
                            if (!parse_bool(value, bv))
                            {
                                fprintf(stderr, "Cannot parse %s as a boolean value.\n", value);
                                return false;
                            }
                        a.bool_values.push_back(bv);
                        break;
                        case argument::string:
                            a.string_values.push_back(value);
                        break;
                        case argument::number:
                            if (!parse_uint(value, nv))
                            {
                                fprintf(stderr, "Cannot parse %s as a number.\n", value);
                                return false;
                            }
                        a.number_values.push_back(nv);
                        break;
                    }
                    break;
                }
            }
            if (!found)
            {
                fprintf(stderr, "Unknown argument name: %s\n", argv[i]);
                return false;
            }
        }
    }
    return true;
}


bool validate_arguments()
{
    for (auto const &a : arguments)
    {
        unsigned n = a.bool_values.size() + a.string_values.size() + a.number_values.size();
        if (n < a.min_count)
        {
            fprintf(stderr, "Too few arguments provided for %s (%s).\n%s\n", a.name, a.abbreviation, a.description);
            return false;
        }
        if (n > a.max_count)
        {
            fprintf(stderr, "Too many arguments provided for %s (%s).\n%s\n", a.name, a.abbreviation, a.description);
            return false;
        }
    }
    files = arguments[ARG_INPUT].string_values;
    output = arguments[ARG_OUTPUT].string_values.back();
    refresh_count = arguments[ARG_REFRESH].number_values.back();
    iteration_count = arguments[ARG_ITERATIONS].number_values.back();
    if (arguments[ARG_ALGORITHM].string_values.size() != 0u)
    {
        std::string const &chosen_name = arguments[ARG_ALGORITHM].string_values.front();
        for (auto const &a : algorithms)
        {
            if (a.name == chosen_name)
            {
                chosen = &a;
                break;
            }
        }
        if (!chosen)
        {
            fprintf(stderr, "Unknown algorithm: %s\n", chosen_name.c_str());
            return false;
        }
    }
    if (refresh_count > 100000000u)
    {
        fputs("The number of extra queries must be at most 100000000.\n", stderr);
        return false;
    }
    if (iteration_count > 100000000u)
    {
        fputs("The number of iterations must be at most 100000000.\n", stderr);
        return false;
    }
    return true;
}

void run_isolated_test(std::string const &dataset)
{
    FILE *fq = fopen(dataset.c_str(), "r");
    helper::file_handler::layout_file matrices{ fq };
    fclose(fq);
    if (!matrices.valid)
    {
        fprintf(stderr, "Matrix file of %s is invalid.\n", dataset.c_str());
        return;
    }
    algorithm_profile::multiply_delegate mult = chosen->multiply;
    int **dest;
    for (unsigned i = refresh_count; i; --i){
        mult((int const **)matrices.layoutA, (int const **)matrices.layoutB, matrices.layout_m, matrices.layout_n, matrices.layout_p, dest);
        helper::matrix::destroy_matrix(dest);
    }
    for (unsigned i = iteration_count; i; --i){
        mult((int const **)matrices.layoutA, (int const **)matrices.layoutB, matrices.layout_m, matrices.layout_n, matrices.layout_p, dest);
        helper::matrix::destroy_matrix(dest);
    }
}

void run_test(std::string const &dataset, FILE *f)
{
    FILE *fq = fopen(dataset.c_str(), "r");
    helper::file_handler::layout_file matrices{ fq };
    fclose(fq);
    if (!matrices.valid)
    {
        for (auto const &a : algorithms)
            printf("N/A\t");
        puts(dataset.c_str());
        return;
    }
    int **dest;
    std::string last_layout;
    for (auto const &a : algorithms)
    {
        algorithm_profile::multiply_delegate mult = a.multiply;
        for (unsigned i = refresh_count; i; --i){
            mult((int const **)matrices.layoutA, (int const **)matrices.layoutB, matrices.layout_m, matrices.layout_n, matrices.layout_p, dest);
            helper::matrix::destroy_matrix(dest);
        }
        auto start_tick = clocking::ticks();
        for (unsigned i = iteration_count; i; --i){
            mult((int const **)matrices.layoutA, (int const **)matrices.layoutB, matrices.layout_m, matrices.layout_n, matrices.layout_p, dest);
            helper::matrix::destroy_matrix(dest);
        }
        auto end_tick = clocking::ticks();
        double t = (end_tick - start_tick) / (double)iteration_count;
        fprintf(f, "%f ", t);
        printf("%.0f\t", t);
    }
    fprintf(f, "%d %d %d\n", matrices.layout_m, matrices.layout_n, matrices.layout_p);
    puts(dataset.c_str());
}

void call_gnuplot()
{
    constexpr unsigned algo_count = sizeof(algorithms) / sizeof(algorithms[0]);
    FILE *fplot = fopen((output + ".gnuplot").c_str(), "w");
    fprintf(fplot,
            "set term png\n"
                "set output '%s.png'\n"
                "set ylabel 'cycles / query' rotate by 90\n"
                "set xlabel 'log(#[elements])'\n"
                "set key autotitle columnhead\n"
                "set title 'Time per query to Pred(x)'\n"
                "set key left top\n"
                "plot for [col=1:%d] '%s.data' using %d:col with lines\n",
            output.c_str(), algo_count, output.c_str(), algo_count + 1);
    fclose(fplot);
    int gnuplot_ret = system(("gnuplot " + output + ".gnuplot").c_str());
    if (gnuplot_ret)
        fprintf(stderr, "Call to gnuplot failed with code %d.\n", gnuplot_ret);
}
