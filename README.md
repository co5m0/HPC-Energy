# HPC-Energy

Repository for HPC project

## Run the `src/` programs

1. Go to `lib/` folder and run the `make` command
2. Go inside the topic folder in `src/` ("matrix_mul" or "reduce")
3. Compile the program with:
   `gcc -o main.out ../../lib/rapllib.o <source_file>.c -lm -fopenmp -O2`
4. Run `main.out` as **sudo**: `sudo ./main.out <param1> <param2> ...`

**NOTE: MAKEFILES INSIDE `src/` FOLDERS DOESN'T WORKS**

## Test CPU specs

|                   |       |
| ----------------- | ----- |
| # of CPU Cores    | 6     |
| # of Threads      | 12    |
| Total L1 Cache    | 576KB |
| Total L2 Cache    | 3MB   |
| Total L3 Cache    | 16MB  |
| Default TDP / TDP | 65W   |
| AES, AVX2, FMA3   | YES   |
