#include<cstdlib>
#include<cstring>
#include<ctime>
#include<iostream>
#include<fstream>

#include "papi.h"
#include "../../pcm/cpucounters.h"

#include "../src/naive.hpp"
#include "../src/oblivious.hpp"
#include "../src/oblivious_s.hpp"
#include "../src/oblivious_cores.hpp"
#include "../src/helper.hpp"
#include "../src/naive_flip.hpp"
#include "../src/oblivious_s_flip.hpp"
#include "../src/tiled.hpp"
#include "../../pcm/types.h"
#include "../src/tiled_flip.hpp"

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
        {"core",       "c", "Sets the execution core index",                                                                               0u, 4u,   argument::number, {}, {},         {4u}},
    };

constexpr int ARG_INPUT = 0;
constexpr int ARG_OUTPUT = 1;
constexpr int ARG_REFRESH = 2;
constexpr int ARG_ITERATIONS = 3;
constexpr int ARG_CORE = 4;

std::vector<std::string> files;
std::string output;
unsigned refresh_count;
unsigned iteration_count;
unsigned core;

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
    unsigned max_input;

} algorithms[] =
    {
        {"naive:1", matmul::naive::multiply, matmul::naive::build, 0, "nai1", false, 3500},
        {"naive:fl", matmul::naive_flip::multiply, matmul::naive_flip::build, 0, "nai.fl", true, 50000},
//        {"naive:2", matmul::naive::multiply, matmul::naive::build, 2, "nai2", false},
//        {"naive:4", matmul::naive::multiply, matmul::naive::build, 4, "nai4", false},
//        {"naive:8", matmul::naive::multiply, matmul::naive::build, 8, "nai8", false},
//        {"naive:16", matmul::naive::multiply, matmul::naive::build, 16, "nai16", false},
//        {"naive:32", matmul::naive::multiply, matmul::naive::build, 16, "nai32", false},
//        {"tiled:32", matmul::tiled::multiply, matmul::tiled::build, 32, "tiled.32", true, 25000},
//        {"tiled:50", matmul::tiled::multiply, matmul::tiled::build, 50, "tiled.50", true, 25000},
//        {"tiled:70", matmul::tiled::multiply, matmul::tiled::build, 75, "tiled.75", true, 25000},
//        {"tiled:145", matmul::tiled::multiply, matmul::tiled::build, 145, "tiled.145", true, 25000},
//        {"tiled:300", matmul::tiled::multiply, matmul::tiled::build, 300, "tiled.300", true, 25000},
//        {"tiled:fl:20", matmul::tiled_flip::multiply, matmul::tiled_flip::build, 20, "tiled.fl.20", true, 25000},
//        {"tiled:fl:50", matmul::tiled_flip::multiply, matmul::tiled_flip::build, 20, "tiled.fl.50", true, 25000},
        {"tiled:fl:140", matmul::tiled_flip::multiply, matmul::tiled_flip::build, 20, "tiled.fl.140", true, 25000},
//        {"obl:2", matmul::oblivious::multiply, matmul::oblivious::build, 2, "2", false, 1500},
//        {"obl:10", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 10, "10", false},
//        {"obl:40", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 40, "40", false},
//        {"obl:130", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 130, "obl.130", false, 25000},
//        {"obl:145", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 145, "obl.145", false, 25000},
        {"obl:160", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 160, "obl.160", false, 25000},
//        {"obl:180", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 180, "obl.180", false, 25000},
//        {"obl:640", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 640, "640", false},
//        {"obl:1280", matmul::oblivious_s::multiply, matmul::oblivious_s::build, 1280, "1280", false},
//        {"obl:fl:8", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 8, "nai.fl.8", true},
//        {"obl:fl:16", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 16, "nai.fl.16", true},
//        {"obl:fl:32", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 32, "nai.fl.32", true},
//        {"obl:fl:64", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 64, "nai.fl.64", true},
//        {"obl:fl:128", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 128, "nai.fl.128", true},
//        {"obl:fl:256", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 256, "nai.fl.256", true},
//        {"obl:fl:512", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 512, "obl.fl.512", true},
        {"obl:fl:1024", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 1024, "obl.fl.1024", true,
            50000},
//        {"obl:fl:2048", matmul::oblivious_s_flip::multiply, matmul::oblivious_s_flip::build, 2048, "obl.fl.2048", true},
//        {"obl_c:16", matmul::oblivious_c::multiply, matmul::oblivious_c::build, 16, "16", false},
//        {"obl_c:8", matmul::oblivious_c::multiply, matmul::oblivious_c::build, 8, "8", false},
//        {"obl_c:4", matmul::oblivious_c::multiply, matmul::oblivious_c::build, 4, "4", false},
//        {"obl_c:2", matmul::oblivious_c::multiply, matmul::oblivious_c::build, 2, "2", false},
//        {"obl_c:1", matmul::oblivious_c::multiply, matmul::oblivious_c::build, 1, "1", false},
    };

algorithm_profile const *chosen;

struct pcm_hardware_counter {
    uint64 umask;
    uint64 event_select;
    std::string short_description;
    std::string description;
};
pcm_hardware_counter pcm_hdw_counters[] = {
    {0x08, 0xD1,            "L1_MISS",       "Retired load uops misses in L1 cache as data sources."},
    {0x10, 0xD1,            "L2_MISS",       "Miss in mid-level (L2) cache. Excludes Unknown data-source."},
    {0x01, 0x08,            "TLB_LM",       "DTLB_LOAD_MISSES.MISS_CAUSES_A_WALK:Load misses in all DTLB levels that cause page walks"},
//    {0x02, 0x08,            "TLB_DLM",      "Demand load Miss in all translation lookaside buffer (TLB) levels causes a page walk that completes (4K)."},
//    {0x10, 0x49,            "TLB_LCY",      "DTLB_STORE_MISSES.WALK_DURATION:Cycles when PMH is busy with page walks"},
//    {0x20, 0x49,            "TLB_L2C_4k",   "DTLB_STORE_MISSES.STLB_HIT_4K:Store misses that miss the  DTLB and hit the STLB (4K)."},
//    {0x40, 0x49,            "TLB_L2C_2m",   "DTLB_STORE_MISSES.STLB_HIT_2M:Store misses that miss the  DTLB and hit the STLB (2M)."},
};

int pcm_cnt = sizeof(pcm_hdw_counters) / sizeof(pcm_hdw_counters[0]);
uint32 pcm_in_use = (uint32) std::min(pcm_cnt, 4);

std::string file_name_for_algorithm(algorithm_profile algorithm, std::string ending = ".data");

void print_usage();

bool parse_arguments(int argc, char **argv);

bool validate_arguments();

void run_test(std::string const &dataset, PCM *m);

EventSelectRegister regs[4];

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
    for (unsigned i = 0; i < pcm_in_use; i++) {
        header += pcm_hdw_counters[i].short_description + " ";
    }
    header += "Build_T Mult_T Total_T l2_SYS_MISS l3_SYS_MISS m";
    for (auto const a : algorithms) {
        std::ofstream result_file;
        result_file.open(file_name_for_algorithm(a));
        result_file << header << std::endl;
        result_file.close();
    }

    EventSelectRegister def_event_select_reg;
    def_event_select_reg.value = 0;
    def_event_select_reg.fields.usr = 1;
    def_event_select_reg.fields.os = 1;
    def_event_select_reg.fields.enable = 1;

    PCM::ExtendedCustomCoreEventDescription conf;
    conf.fixedCfg = NULL; // default
    conf.nGPCounters = pcm_in_use;

    conf.gpCounterCfg = regs;
    for (int i = 0; i < pcm_in_use; ++i)
        regs[i] = def_event_select_reg;

    for (int i = 0; i < pcm_in_use; i++) {
        pcm_hardware_counter c = pcm_hdw_counters[i];
        regs[i].fields.event_select = c.event_select;
        regs[i].fields.umask = c.umask;
    }

    PCM *m = PCM::getInstance();
    PCM::ErrorCode status = m->program(PCM::EXT_CUSTOM_CORE_EVENTS, &conf);
    switch (status) {
        case PCM::Success:
            break;
        case PCM::MSRAccessDenied:
            std::cerr << "Access to Processor Counter Monitor has denied (no MSR or PCI CFG space access)."
                      << std::endl;
        exit(EXIT_FAILURE);
        case PCM::PMUBusy:
            std::cerr
                << "Access to Processor Counter Monitor has denied (Performance Monitoring Unit is occupied by other application). Try to stop the application that uses PMU."
                << std::endl;
        std::cerr << "Alternatively you can try to reset PMU configuration at your own risk. Try to reset? (y/n)"
                  << std::endl;
        char yn;
        std::cin >> yn;
        if ('y' == yn) {
            m->resetPMU();
            std::cerr << "PMU configuration has been reset. Try to rerun the program again." << std::endl;
        }
        exit(EXIT_FAILURE);
        default:
            std::cerr << "Access to Processor Counter Monitor has denied (Unknown error)." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << header << std::endl;
    for (auto const &file : files)
        run_test(file, m);

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
    for (int i = 0; i < pcm_in_use; i++) {
        std::cout << pcm_hdw_counters[i].short_description << " ";
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
    core = arguments[ARG_CORE].number_values.back();
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

void run_test(std::string const &dataset, PCM *m) {
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

    const uint32 ncores = m->getNumCores();
    uint64 BeforeTime = 0, AfterTime = 0;
    SystemCounterState SysBeforeState, SysAfterState;
    std::vector<CoreCounterState> BeforeState, AfterState;
    std::vector<SocketCounterState> DummySocketStates;

    helper::matrix::initialize_matrix(dest, matrices.layout_m, matrices.layout_p);
    bool equal_iter = iteration_count % 2 == 0;
    for (auto const &a : algorithms) {
        if(a.max_input < matrices.layout_m){
            std::cout << " # " << a.max_input << " < " << matrices.layout_m << " " << a.name << " skipping" << std::endl;
            continue;
        }

        uint64 l2_acc = 0, l3_acc = 0;
        uint64 transpose_acc = 0, transpose_start = 0, transpose_stop = 0;

        algorithm_profile::build_delegate build = a.build;
        unsigned reps = B_flipped ? (equal_iter ? iteration_count : iteration_count + 1)
                                  : (equal_iter ? iteration_count + 1 : iteration_count);
        transpose_start = m->getTickCount(1000000, core); // measure in ms on core we are running on.
        for(unsigned i = 0; i < reps; i++)
            B_flipped = build(matrices.layoutA, matrices.layoutB, matrices.layout_m, matrices.layout_n,
                          matrices.layout_p);
        transpose_stop = m->getTickCount(1000000, core);
        transpose_acc = (transpose_stop - transpose_start) / reps;

        algorithm_profile::multiply_delegate mult = a.multiply;
        uint64 acc_time = 0;
        uint64 acc_counts[ncores][pcm_in_use];

        for(unsigned i = 0; i < ncores; i++)
            for(unsigned j = 0; j < pcm_in_use; j++)
                acc_counts[i][j] = 0;

        for (unsigned i = refresh_count; i; --i) {
            std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);
            mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
                 matrices.layout_p, dest, a.option);
        }

        for (unsigned i = iteration_count; i; --i) {
            std::memset(dest[0], 0, sizeof(int) * matrices.layout_m * matrices.layout_p);

            BeforeTime = m->getTickCount(1000000, core);
            m->getAllCounterStates(SysBeforeState, DummySocketStates, BeforeState);

            mult((int const **) matrices.layoutA, (int const **) matrices.layoutB, matrices.layout_m, matrices.layout_n,
                 matrices.layout_p, dest, a.option);

            m->getAllCounterStates(SysAfterState, DummySocketStates, AfterState);
            AfterTime = m->getTickCount(1000000, core);

            for (uint32 j = 0; j < ncores; j++) {
                for (uint32 k = 0; k < pcm_in_use; k++) {
                    acc_counts[j][k] += getNumberOfCustomEvents(k, BeforeState[j], AfterState[j]);
                }
            }
            acc_time += AfterTime - BeforeTime;
            l2_acc += getL2CacheMisses(SysBeforeState, SysAfterState);
            l3_acc += getL3CacheMisses(SysBeforeState, SysAfterState);
        }

        l2_acc /= iteration_count;
        l3_acc /= iteration_count;

        for (uint32 i = 0; i < ncores; i++){
            for (uint32 j = 0; j < pcm_in_use; j++){
                acc_counts[i][j] /= iteration_count;
            }
        }
        acc_time /= iteration_count;
        std::ofstream result_file;
        result_file.open(file_name_for_algorithm(a), std::ios::app);

        unsigned max_idx = 0;
        if(core == 4){
            uint64 max_value = 0;

            for(unsigned i = 0; i < pcm_in_use; i++){
                if(acc_counts[i][0] > max_value){
                    max_value = acc_counts[i][0];
                    max_idx = i;
                }
            }
        } else {
            max_idx = core;
        }

        for (unsigned i = 0u; i < pcm_in_use; i ++){
            result_file << acc_counts[max_idx][i] << "  ";
            std::cout << acc_counts[max_idx][i] << "  ";
        }

        result_file << transpose_acc << "  ";
        result_file << acc_time << " " << transpose_acc + acc_time  << "  " << l2_acc << "  " << l3_acc << "  " << matrices.layout_m << " " << a.name << std::endl;
        result_file.close();
        std::cout << transpose_acc << "  ";
        std::cout << acc_time << " " << transpose_acc + acc_time  << "  " << l2_acc << "  " << l3_acc << "  " << matrices.layout_m << " " << a.name << std::endl;

        std::swap(BeforeTime, AfterTime);
        std::swap(BeforeState, AfterState);
        std::swap(SysBeforeState, SysAfterState);

    }
    helper::matrix::destroy_matrix(dest);
}
