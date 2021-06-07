# `lib/` folder

Contains the 2 header files and their implementation to proper read energy throughout the rapl interface and for print the benchmark results in a _.csv_ file:

- [`rapllib.h`](./rapllib.h) expose the _Rapl_info_ and _Rapl_power_info_ structures and the methods for reading the energy
- [`rapllib.c`](./rapllib.c) implement the header functions
- [`print.h`](./print.h) expose and implement the function for writing a _.csv_ file with the benchmark results
- [`test.c`](./test.c) test the libraries functions
- [`Makefile`](./Makefile) makes two target: the object file for _rapllib_ (`rapllib.o`), _main.out_ an executable to test if library object runs without problems
