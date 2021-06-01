#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "rapllib.h"

int main(int argc, char const *argv[]) {
    /* code */
    // father -> write 0 when has finished the computation
    // children -> read until father don't write 0
    int comm_pipe[2];
    int sendToPipe = 1;
    pid_t p;

    Rapl_info rapl = new_rapl_info();
    Rapl_power_info rapl_power = new_rapl_power_info();

    if (pipe(comm_pipe) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    if (rapl != NULL && rapl_power != NULL) {
        int is_detected_model = detect_cpu(rapl);
        int is_deceted_package = detect_packages(rapl);
        // int is_rapl_msr = rapl_msr(0, rapl);
        // int start = rapl_msr_start(rapl);
        // int stop = rapl_msr_stop(rapl);
        p = fork();
        if (p < 0) {
            fprintf(stderr, "fork Failed");
            return 1;
        } else if (p > 0) {
            // father

            int is_rapl_sysfs = rapl_sysfs(rapl);
            int start = rapl_sysfs_start(rapl);
            sleep(1);
            int stop = rapl_sysfs_stop(rapl);
            // write(comm_pipe[1], 0, sizeof(int));
            sendToPipe = -1;
            write(comm_pipe[1], &sendToPipe, sizeof(int));
            close(comm_pipe[1]);
            wait(0);
            printf("\n\n----- LOG ALL -----\n\n");
        } else {
            // children
            rapl_power_sysfs(rapl, rapl_power);
            read_power(rapl, rapl_power, 500, 2, "test");
        }
    }
    return 0;
}
