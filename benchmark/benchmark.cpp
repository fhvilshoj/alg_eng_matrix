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
#include "../src/naive_flip.hpp"
#include "../src/oblivious_s_flip.hpp"
#include "../src/tiled_flip.hpp"
#include "../src/tiled.hpp"

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
        {"refresh",    "r", "Sets the number of extra iterations to run before measuring the time. Defaults to 0, at most 1.000.000.000.", 1u, 2u,   argument::number, {}, {},         {2u}},
        {"iterations", "l", "Sets the number of iterations to run when measuring the time. Defaults to 1000, at most 1.000.000.000.",      1u, 2u,   argument::number, {}, {},         {5u}},
    };

constexpr int ARG_INPUT = 0;
constexpr int ARG_OUTPUT = 1;
constexpr int ARG_REFRESH = 2;
constexpr int ARG_ITERATIONS = 3;

std::vector<std::string> files;
std::string output;
unsigned refresh_count;
unsigned iteration_count;


struct algorithm_profile {
    typedef void (*multiply_delegate)(int const **A, int const **B, unsigned const m, unsigned const n,
                                      unsigned const p, int **&destination, unsigned const option);

    typedef bool (*build_delegate)(int **&A, int **&B, unsigned const m, unsigned const n, unsigned const p);

    char const *name;
    multiply_delegate multiply;
    build_delegate build;
    unsigned const option;
    std::string short_name;
    bool mutates_input;

} algorithms[] =
    {
//        {"obl:1280",    matmul::oblivious_s::multiply,      matmul::oblivious_s::build,     1280, "1280",         false},
//        {"obl:640",     matmul::oblivious_s::multiply,      matmul::oblivious_s::build,      640, "640",          false},
//        {"obl:160",     matmul::oblivious_s::multiply,      matmul::oblivious_s::build,      160, "160",          false},
//        {"obl:2",       matmul::oblivious::multiply,        matmul::oblivious::build,          2, "2",            false},
//        {"obl:40",      matmul::oblivious_s::multiply,      matmul::oblivious_s::build,       40, "40",           false},
//        {"obl:10",      matmul::oblivious_s::multiply,      matmul::oblivious_s::build,       10, "10",           false},
//        {"obl:1",       matmul::oblivious::multiply,        matmul::oblivious::build,          0, "0",            false},
//        { "obl_c:16",   matmul::oblivious_c::multiply,      matmul::oblivious_c::build,       16, "16",           false},
//        { "obl_c:8",    matmul::oblivious_c::multiply,      matmul::oblivious_c::build,        8, "8",            false},
//        { "obl_c:4",    matmul::oblivious_c::multiply,      matmul::oblivious_c::build,        4, "4",            false},
//        { "obl_c:2",    matmul::oblivious_c::multiply,      matmul::oblivious_c::build,        2, "2",            false},
//        { "obl_c:1",    matmul::oblivious_c::multiply,      matmul::oblivious_c::build,        1, "1",            false},
//        { "obl:fl:8",   matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build,   8, "nai.fl.8",     true},
//        { "obl:fl:16",  matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build,  16, "nai.fl.16",    true},
//        { "obl:fl:32",  matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build,  32, "nai.fl.32",    true},
//        { "obl:fl:64",  matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build,  64, "nai.fl.64",    true},
//        { "obl:fl:128", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 128, "nai.fl.128",   true},
//        { "obl:fl:256", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 256, "nai.fl.256",   true},
//        {"obl:fl:512",  matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 512, "obl.fl.512",   true},
//        {"obl:fl:1024", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build,1024, "obl.fl.1024",  true},
//        {"obl:fl:2048", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build,2048, "obl.fl.2048",  true},

        {"naive:fl",    matmul::naive_flip::multiply, matmul::naive_flip::build, 0, "nai.fl", true},
//        { "naive:1",  matmul::naive::multiply, matmul::naive::build, 0,  "nai1",  false},
//        { "naive:2",  matmul::naive::multiply, matmul::naive::build, 2,  "nai2",  false},
//        { "naive:4",  matmul::naive::multiply, matmul::naive::build, 4,  "nai4",  false},
//        { "naive:8",  matmul::naive::multiply, matmul::naive::build, 8,  "nai8",  false},
//        { "naive:16", matmul::naive::multiply, matmul::naive::build, 16, "nai16", false},
//        { "naive:32", matmul::naive::multiply, matmul::naive::build, 16, "nai32", false},

//        {"tiled:20",     matmul::tiled::multiply,      matmul::tiled::build,      20, "tiled.fl.20", false},
//        {"tiled:32",     matmul::tiled::multiply,      matmul::tiled::build,      20, "tiled.fl.32", false},
//        {"tiled:50",     matmul::tiled::multiply,      matmul::tiled::build,      20, "tiled.fl.50", false},
//        {"tiled:145",     matmul::tiled::multiply,      matmul::tiled::build,      20, "tiled.fl.145", false},
        {"tiled:fl:20",  matmul::tiled_flip::multiply, matmul::tiled_flip::build, 20, "tiled.fl.20", true},
        {"tiled:fl:50",  matmul::tiled_flip::multiply, matmul::tiled_flip::build, 50, "tiled.fl.50", true},
        {"tiled:fl:140", matmul::tiled_flip::multiply, matmul::tiled_flip::build, 140, "tiled.fl.140", true},
    };

algorithm_profile const *chosen;

struct hardware_counter {
    int counter;
    std::string description;
    std::string short_description;
};

hardware_counter hdw_counters[] = {
//    {PAPI_TLB_TL,  "Total translation lookaside buffer misses", "TLB_TL"},
    {PAPI_L1_TCM,  "Level 1 total cache misses",                "L1_TCM"},
    {PAPI_L2_TCM,  "Level 2 total cache misses",                "L2_TCM"},
    {PAPI_L3_TCM,  "Level 3 total cache misses",                "L3_TCM"},
    {PAPI_TOT_CYC, "Total cycles executed",                     "TOT_CYC"},
    {PAPI_BR_CN,   "Conditional branch instructions executed",  "BR_CN"},
    {PAPI_BR_MSP,  "Conditional branch instructions mispred",   "BR_MSP"},
};

//PAPI_EINVAL

int hdw_counters_cnt = sizeof(hdw_counters) / sizeof(hdw_counters[0]);

std::string file_name_for_algorithm(algorithm_profile algorithm, std::string ending = ".data");

void print_usage();

bool parse_arguments(int argc, char **argv);

bool validate_arguments();

void run_test(std::string const &dataset);

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

void run_test(std::string const &dataset) {
    std::cout << std::endl << dataset << std::endl;
    bool B_flipped = false;

    FILE *fq = fopen(dataset.c_str(), "r");
    helper::file_handler::layout_file matrices{fq};
    fclose(fq);
    if (!matrices.valid) {
        for (auto const &a : algorithms)
            printf("N/A\t");
        puts(dataset.c_str());
        return;
    }

    int **dest;
    int events[hdw_counters_cnt];
    for (int i = 0; i < hdw_counters_cnt; i++) {
        events[i] = hdw_counters[i].counter;
    }

    helper::matrix::initialize_matrix(dest, matrices.layout_m, matrices.layout_p);
    for (auto const &a : algorithms) {

        algorithm_profile::build_delegate build = a.build;
        if (!B_flipped) {
            B_flipped = build(matrices.layoutA, matrices.layoutB, matrices.layout_m, matrices.layout_n,
                              matrices.layout_p);
        }

        algorithm_profile::multiply_delegate mult = a.multiply;

        long long counts[hdw_counters_cnt] = {};
        long long stoppers[3] = {};

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
