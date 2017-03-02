#include<cstdlib>
#include<cstring>
#include<ctime>
#include<iostream>
#include<fstream>

#include "papi.h"

#include "../src/naive.hpp"
#include "../src/oblivious.hpp"
#include "../src/oblivious_s.hpp"
#include "../src/oblivious_cores.hpp"
#include "../src/helper.hpp"

#ifdef __GNUC__

#include<x86intrin.h>

namespace clocking {
    inline unsigned long long ticks() {
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


struct argument {
    char const *name;
    char const *abbreviation;
    char const *description;
    unsigned min_count, max_count;
    enum {
        boolean, string, number
    } type;
    std::vector<bool> bool_values;
    std::vector<std::string> string_values;
    std::vector<unsigned> number_values;

    bool is(char const *testee) const {
        return std::strcmp(testee, abbreviation) == 0
               || std::strcmp(testee, name) == 0;
    }
} arguments[] =
    {
        {"input",      "i", "Sets the input query file. Multiple ones allowed.",                                                           1u, 128u, argument::string},
        {"output",     "o", "Sets the output file name. Defaults to 'result'. This argument is ignored if algorithm is set.",              1u, 2u,   argument::string, {}, {"result"}, {}},
        {"refresh",    "r", "Sets the number of extra iterations to run before measuring the time. Defaults to 0, at most 1.000.000.000.", 1u, 2u,   argument::number, {}, {},         {0u}},
        {"algorithm",  "a", "Sets the only algorithm to run. This is for isolated tests.",                                                 0u, 1u,   argument::string, {}, {},         {}},
        {"iterations", "l", "Sets the number of iterations to run when measuring the time. Defaults to 1000, at most 1.000.000.000.",      1u, 2u,   argument::number, {}, {},         {1000u}},
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


struct algorithm_profile {
    typedef void (*multiply_delegate)(int const **A, int const **B, unsigned const m, unsigned const n,
                                      unsigned const p, int **&destination, unsigned const option);

    char const *name;
    multiply_delegate multiply;
    unsigned const option;
    std::string short_name;

} algorithms[] =
    {
        /*{"obl:1280", matmul::oblivious_s::multiply, 1280, "1280"},
            {"obl:640", matmul::oblivious_s::multiply, 640, "640"},
            {"obl:160", matmul::oblivious_s::multiply, 160, "160"},
            {"obl:40", matmul::oblivious_s::multiply, 40, "40"},
            {"obl:10", matmul::oblivious_s::multiply, 10, "10"},
            {"obl:2",   matmul::oblivious::multiply, 2, "2"}
            {"obl:1",   matmul::oblivious::multiply, 0, "0"},
            */
    { "obl_c:16", matmul::oblivious_c::multiply, 16, "16" },
    { "obl_c:8", matmul::oblivious_c::multiply, 8, "8" },
    { "obl_c:4", matmul::oblivious_c::multiply, 4, "4" },
    { "obl_c:2", matmul::oblivious_c::multiply, 2, "2" },
    { "obl_c:1", matmul::oblivious_c::multiply, 1, "1" },
    { "naive:1", matmul::naive::multiply, 1, "nai1" }
        /*,
    { "naive:2", matmul::naive::multiply, 2, "nai2" },
    { "naive:4", matmul::naive::multiply, 4, "nai4" },
    { "naive:8", matmul::naive::multiply, 8, "nai8" },
    { "naive:16", matmul::naive::multiply, 16, "nai16" },
    { "naive:32", matmul::naive::multiply, 16, "nai32" },*/
    };

algorithm_profile const *chosen;

struct hardware_counter {
    int counter;
    std::string description;
    std::string short_description;
};

hardware_counter hdw_counters[] = {
    {PAPI_L1_TCM,  "Level 1 total cache misses",                "L1_TCM"},
    {PAPI_L2_TCM,  "Level 2 total cache misses",                "L2_TCM"},
    {PAPI_L3_TCM,  "Level 3 total cache misses",                "L3_TCM"},
    {PAPI_TOT_CYC, "Total cycles executed",                     "TOT_CYC"},
    {PAPI_BR_CN,   "Conditional branch instructions executed",  "BR_CN"},
    {PAPI_BR_MSP,  "Conditional branch instructions mispred",   "BR_MSP"},
    {PAPI_TLB_TL,  "Total translation lookaside buffer misses", "TLB_TL"},
};

//PAPI_EINVAL

int hdw_counters_cnt = sizeof(hdw_counters) / sizeof(hdw_counters[0]);

std::string file_name_for_algorithm(algorithm_profile algorithm, std::string ending = ".data");

void print_usage();

void print_meassures();

bool parse_arguments(int argc, char **argv);

bool validate_arguments();

void run_isolated_test(std::string const &dataset);

void run_test(std::string const &dataset);

void call_gnuplot(algorithm_profile algorithm);

int main(int argc, char **argv) {
    std::cout << "Lets benchmark!" << std::endl;

    if (argc < 2) {
        print_usage();
        return 1;
    }
    if (!parse_arguments(argc, argv)) {
        return 2;
    }
    if (!validate_arguments()) {
        return 2;
    }
    if (chosen) {
        print_meassures();
        for (auto const &file : files) {
            run_isolated_test(file);
        }
    } else {
        std::string header = "";
        for (unsigned i = 0; i < hdw_counters_cnt; i++) {
            header += hdw_counters[i].short_description + " ";
        }
        header += "Time m";
        for (auto const a : algorithms) {
            std::ofstream result_file;
            result_file.open(file_name_for_algorithm(a));
            result_file << header << std::endl;
            result_file.close();
        }
        for (auto const &file : files)
            run_test(file);
        for (auto const a : algorithms) {
            call_gnuplot(a);
        }
    }
    return 0;
}

std::string file_name_for_algorithm(algorithm_profile algorithm, std::string ending) {
    return output + "_" + algorithm.short_name + ending;
}

void print_usage() {
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

void print_meassures() {
    for (int i = 0; i < hdw_counters_cnt; i++) {
        std::cout << hdw_counters[i].short_description << " ";
    }
    std::cout << "filename" << std::endl;
}

bool parse_uint(char const *str, unsigned &result) {
    if (!str)
        return false;
    unsigned x = 0u;
    for (unsigned digit; *str; ++str) {
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

bool parse_bool(char const *str, bool &result) {
    static char const *const bool_true[] =
        {
            "true", "t", "on", "yes", "y", nullptr
        };
    static char const *const bool_false[] =
        {
            "false", "f", "off", "no", "n", nullptr
        };
    for (char const *const *p = bool_true; *p; ++p)
        if (std::strcmp(*p, str) == 0) {
            result = true;
            return true;
        }
    for (char const *const *p = bool_false; *p; ++p)
        if (std::strcmp(*p, str) == 0) {
            result = false;
            return true;
        }
    return false;
}

bool parse_arguments(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return false;
        }
        ++argv[i];
        char *value = std::strstr(argv[i], ":");
        /* switch */
        if (!value) {
            for (auto &a : arguments)
                if (a.type == argument::boolean && a.is(argv[i]))
                    a.bool_values.push_back(true);
        } else {
            *value++ = '\0';
            bool found = false;
            for (auto &a : arguments) {
                if (a.is(argv[i])) {
                    found = true;
                    bool bv;
                    unsigned nv;
                    switch (a.type) {
                        case argument::boolean:
                            if (!parse_bool(value, bv)) {
                                fprintf(stderr, "Cannot parse %s as a boolean value.\n", value);
                                return false;
                            }
                        a.bool_values.push_back(bv);
                        break;
                        case argument::string:
                            a.string_values.push_back(value);
                        break;
                        case argument::number:
                            if (!parse_uint(value, nv)) {
                                fprintf(stderr, "Cannot parse %s as a number.\n", value);
                                return false;
                            }
                        a.number_values.push_back(nv);
                        break;
                    }
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "Unknown argument name: %s\n", argv[i]);
                return false;
            }
        }
    }
    return true;
}


bool validate_arguments() {
    for (auto const &a : arguments) {
        unsigned n = a.bool_values.size() + a.string_values.size() + a.number_values.size();
        if (n < a.min_count) {
            fprintf(stderr, "Too few arguments provided for %s (%s).\n%s\n", a.name, a.abbreviation, a.description);
            return false;
        }
        if (n > a.max_count) {
            fprintf(stderr, "Too many arguments provided for %s (%s).\n%s\n", a.name, a.abbreviation, a.description);
            return false;
        }
    }
    files = arguments[ARG_INPUT].string_values;
    output = arguments[ARG_OUTPUT].string_values.back();
    refresh_count = arguments[ARG_REFRESH].number_values.back();
    iteration_count = arguments[ARG_ITERATIONS].number_values.back();
    if (arguments[ARG_ALGORITHM].string_values.size() != 0u) {
        std::string const &chosen_name = arguments[ARG_ALGORITHM].string_values.front();
        for (auto const &a : algorithms) {
            if (a.name == chosen_name) {
                chosen = &a;
                break;
            }
        }
        if (!chosen) {
            fprintf(stderr, "Unknown algorithm: %s\n", chosen_name.c_str());
            return false;
        }
    }
    if (refresh_count > 100000000u) {
        fputs("The number of extra queries must be at most 100000000.\n", stderr);
        return false;
    }
    if (iteration_count > 100000000u) {
        fputs("The number of iterations must be at most 100000000.\n", stderr);
        return false;
    }
    return true;
}

void run_isolated_test(std::string const &dataset) {
    FILE *fq = fopen(dataset.c_str(), "r");
    helper::file_handler::layout_file matrices{fq};
    fclose(fq);
    if (!matrices.valid) {
        fprintf(stderr, "Matrix file of %s is invalid.\n", dataset.c_str());
        return;
    }
    long long counters[hdw_counters_cnt] = {};
    long long stoppers[hdw_counters_cnt] = {};

    int cnts[hdw_counters_cnt];
    for (int i = 0; i < hdw_counters_cnt; i++) {
        cnts[i] = hdw_counters[i].counter;
    }

    algorithm_profile::multiply_delegate mult = chosen->multiply;
    int **dest;
    helper::matrix::initialize_matrix(dest, matrices.layout_m, matrices.layout_p);
    for (unsigned i = refresh_count; i; --i) {
        std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);
        mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
             matrices.layout_p, dest, chosen->option);
    }

    for (unsigned i = iteration_count; i; --i) {
        std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);

        int start_counters_res = PAPI_start_counters(cnts, 3);
        if (start_counters_res != PAPI_OK)
            std::cout << "Couldn't start counters" << start_counters_res << std::endl;

        //Actual calculation
        mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
             matrices.layout_p, dest, chosen->option);

        int accum_counters_res = PAPI_accum_counters(counters, 3);
        if (accum_counters_res != PAPI_OK)
            std::cout << "Couldn't accumulate counters" << accum_counters_res << std::endl;

        int stop_counters_res = PAPI_stop_counters(stoppers, 3);
        if (stop_counters_res != PAPI_OK)
            std::cout << "Couldn't stop counters" << stop_counters_res << std::endl;
    }
    for (unsigned i = iteration_count; i; --i) {
        std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);

        int start_counters_res = PAPI_start_counters(cnts + 3, 3);
        if (start_counters_res != PAPI_OK)
            std::cout << "Couldn't start counters" << start_counters_res << std::endl;

        //Actual calculation
        mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
             matrices.layout_p, dest, chosen->option);

        int accum_counters_res = PAPI_accum_counters(counters + 3, 3);
        if (accum_counters_res != PAPI_OK)
            std::cout << "Couldn't accumulate counters" << accum_counters_res << std::endl;

        int stop_counters_res = PAPI_stop_counters(stoppers, 3);
        if (stop_counters_res != PAPI_OK)
            std::cout << "Couldn't stop counters" << stop_counters_res << std::endl;
    }
    helper::matrix::destroy_matrix(dest);

    // Print counter values
    for (int i = 0; i < 6; i++) {
        std::cout << counters[i] / iteration_count << " ";
    }
    std::cout << dataset << std::endl;
}

void run_test(std::string const &dataset) {
    FILE *fq = fopen(dataset.c_str(), "r");
    helper::file_handler::layout_file matrices{fq};
    fclose(fq);
    if (!matrices.valid) {
        for (auto const &a : algorithms)
            printf("N/A\t");
        puts(dataset.c_str());
        return;
    }
    std::cout << std::endl << dataset << std::endl;
    int **dest;

    helper::matrix::initialize_matrix(dest, matrices.layout_m, matrices.layout_p);
    for (auto const &a : algorithms) {
        algorithm_profile::multiply_delegate mult = a.multiply;

        long long counts[hdw_counters_cnt] = {};
        long long stoppers[3] = {};

        int events[hdw_counters_cnt];
        for (int i = 0; i < hdw_counters_cnt; i++) {
            events[i] = hdw_counters[i].counter;
        }

        for (unsigned i = refresh_count; i; --i) {
            std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);
            mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
                 matrices.layout_p, dest, a.option);
        }
        for (unsigned i = iteration_count; i; --i) {
            std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);
            int signal = PAPI_start_counters(events, 3);
            if (signal != PAPI_OK)
                std::cerr << "Failed to start counters with code " << signal << " in file " << dataset << std::endl;

            mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
                 matrices.layout_p, dest, a.option);

            signal = PAPI_accum_counters(counts, 3);
            if (signal != PAPI_OK)
                std::cerr << "Failed to accumulate counters with code " << signal << " in file " << dataset
                          << std::endl;
            signal = PAPI_stop_counters(stoppers, 3);
            if (signal != PAPI_OK)
                std::cerr << "Failed to stop counters with code " << signal << " in file " << dataset << std::endl;
        }
        double acc_time = 0;
        for (unsigned i = iteration_count; i; --i) {
            std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);
            clock_t begin = clock();
            int signal = PAPI_start_counters(events + 3, 3);
            if (signal != PAPI_OK)
                std::cerr << "Failed to start counters with code " << signal << " in file " << dataset << std::endl;

            mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
                 matrices.layout_p, dest, a.option);

            signal = PAPI_accum_counters(counts + 3, 3);
            if (signal != PAPI_OK)
                std::cerr << "Failed to accumulate counters with code " << signal << " in file " << dataset
                          << std::endl;
            signal = PAPI_stop_counters(stoppers, 3);
            if (signal != PAPI_OK)
                std::cerr << "Failed to stop counters with code " << signal << " in file " << dataset << std::endl;
            clock_t end = clock();
            acc_time += double(end - begin) / CLOCKS_PER_SEC;
        }

        acc_time /= iteration_count;
        std::ofstream result_file;
        result_file.open(file_name_for_algorithm(a), std::ios::app);
        for (unsigned i = 0u; i < hdw_counters_cnt; i++) {
            result_file << counts[i] / iteration_count << " ";
            std::cout << counts[i] / iteration_count << " ";
        }
        result_file << acc_time << " " << matrices.layout_m << std::endl;
        result_file.close();
        std::cout << acc_time << " " << matrices.layout_m << " " << a.name << std::endl;
    }
    helper::matrix::destroy_matrix(dest);
}

void call_gnuplot(algorithm_profile algorithm) {
    constexpr unsigned algo_count = sizeof(algorithms) / sizeof(algorithms[0]);
    std::string input_name = file_name_for_algorithm(algorithm);
    std::string output_name = file_name_for_algorithm(algorithm, ".png");
    FILE *fplot = fopen((output + ".gnuplot").c_str(), "w");
    fprintf(fplot,
            "set term png\n"
                "set output '%s'\n"
                "set ylabel 'counts' rotate by 90\n"
                "set xlabel 'size of square matrix'\n"
                "set key autotitle columnhead\n"
                "set title 'Counts per multiplication'\n"
                "set key left top\n"
                "plot for [col=1:%d] '%s' using %d:col with linespoints\n",
            output_name.c_str(), hdw_counters_cnt, input_name.c_str(), hdw_counters_cnt + 1);
    fclose(fplot);
    int gnuplot_ret = system(("gnuplot " + output + ".gnuplot").c_str());
    if (gnuplot_ret)
        fprintf(stderr, "Call to gnuplot failed with code %d.\n", gnuplot_ret);
}
