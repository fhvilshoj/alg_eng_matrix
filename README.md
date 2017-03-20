# AlgEng - Matrix Multiplication

This C++ project is build as a part of the Algorithm 
Engineering course at Aarhus University. 
It includes different includes implementations of 
different alrogithms for multiplying matrices.

## Building

To build the project make a folder called output 
in the root of the project and run cmake and make 
to build it. **Note** this project depends on 
Intels PCM library as well as PAPI. Both are listed 
under dependencies.

The build will generate four executables:

- `testing`: Used to run unittests of all the implementations.
- `datagen`: Used to generate data for the benchmark programs.
- `benchmark`: Benchmarks algorithms using the PAPI library
- `benchmark2`: Benchmarks algorithms using the PCM library

## Testing

The testing executable takes no arguments and runs all tests 
in the test folder

```commandline
> ./testing
```

## Datagen

The datagen executable takes four arguments and generates a 
datafile for two matrices A of size m x n and B og size n x p 
filled with random numbers between -1000 and 1000.
 
 ```commandline
> ./datagen <m> <n> <p> <output-file>
```
## Benchmark

The benchmark executable takes different arguments and benchmarks
all the algorithms using PAPI.

**Arguments**:
- `-r:<refresh>`: is an unsigned int telling how many refresh 
iterations to run before measuring. Default 2.
- `-l:<loop>`: is an unsigned int telling how many iterations
to measure and average over. Default 5.
- `-i:<input-file>`: specifies a file generated by datagen 
to benchmark algorithms against. Multiple files can be specified.
- `-o:<output-file>`: specifies the prefix of the resulting output files.

Example:
```commandline
> sudo taskset -c 0 nice -n -20 ./benchmark -i:data/42_42_42.data -r:5 -l:10 -o:my_run
```

## Benchmark2

The benchmark executable takes different arguments and benchmarks
all the algorithms using PCM.

Arguments are the same as benchmark but with an additional
- `-c:<core>`: specifies the core to measure events from (should 
be the same as set in taskset for linux).

Example:
```commandline
> sudo taskset -c 0 nice -n -20 ./benchmark -i:data/42_42_42.data -r:5 -l:10 -o:my_run -c:0
```

## Dependencies

- [Catch test framework](https://github.com/philsquared/Catch):
    Catch is included in the project and should work out of the box.
- [Intel Performance Counter Monitoring (PCM)](https://github.com/opcm/pcm):
    For PCM to work it has to be cloned from its repository and build. 
    Afterwards it can be linked in the `CMakeLists.txt` file in this project.
    Note the the project is setup to work with Intel® Core™ i7-5600U Processor
    and not guaranteed to work with others.
- [Performance API (PAPI)](http://icl.utk.edu/papi/):
    PAPI works only on Linux and it has to be installed such that `/usr/local/lib/libpapi.a`
    is present on the machine. Installation guide can be found on the webpage.
