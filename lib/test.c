#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "print.h"
#include "rapllib.h"

int main(int argc, char const *argv[]) {
    /* code */
    pid_t proc;

    Rapl_info rapl = new_rapl_info();
    Rapl_power_info rapl_power = new_rapl_power_info();

    detect_cpu(rapl);
    detect_packages(rapl);

    if (rapl != NULL && rapl_power != NULL) {
        // int is_rapl_msr = rapl_msr(0, rapl);
        // int start = rapl_msr_start(rapl);
        // int stop = rapl_msr_stop(rapl);
        proc = fork();
        if (proc < 0) {
            fprintf(stderr, "fork Failed");
            return 1;
        } else if (proc > 0) {
            rapl_sysfs_start(rapl);

            sleep(5);

            rapl_sysfs_stop(rapl);

            wait(0);

            print_file("test.csv", "TEST", 5, 0, rapl_get_energy(rapl), 1);
        } else {  //child
            rapl_power_sysfs(rapl, rapl_power);
            read_power(rapl, rapl_power, 200, 5, "test_power");
        }
    }
    return 0;
}
