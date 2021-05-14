#include <stdio.h>
#include <stdlib.h>

#include "print.h"

int main(int arg, char **argv) {
    double time = 23.5;
    int size = 1024;
    double energy = 20000;
    int nt = 2;
    print_file(__FILE__, time, size, energy, nt);
}