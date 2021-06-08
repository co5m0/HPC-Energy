# Energy​ Consumption ​Analysis​: on a multicore architecture

Repository for HPC project

## Repository structure

- [`lib`](./lib): headers file
- [`src`](./src): source files used for benchmark
- [`results`](./results): csv files with the benchmark results

## Run the `src/` programs

1. Go to `lib/` folder and run the `make` command
2. Go inside the topic folder in `src/` ("_matrix_mul_" or "_reduce_")
3. Compile the program with:
   `gcc -o main.out ../../lib/rapllib.o <source_file>.c -lm -fopenmp -O2`
4. Run `main.out` as **sudo**: `sudo ./main.out <matrix_side_size> <number_of_threads>`

## Test CPU specs

All test in the `results` folder are taken on a NUMA server with these specification:

![Alt text](./topology.png?raw=true "Title")
