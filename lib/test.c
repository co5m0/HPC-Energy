#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rapllib.h"

int main(int argc, char const *argv[]) {
    /* code */

    Rapl_info rapl = new_rapl_info();
    if (rapl != NULL) {
        int is_detected_model = detect_cpu(rapl);
        int is_deceted_package = detect_packages(rapl);
        // int is_rapl_msr = rapl_msr(0, rapl);
        // int start = rapl_msr_start(rapl);
        // int stop = rapl_msr_stop(rapl);
        int is_rapl_sysfs = rapl_sysfs(rapl);
        int start = rapl_sysfs_start(rapl);
        sleep(5);
        int stop = rapl_sysfs_stop(rapl);
        printf("\n\n----- LOG ALL -----\n\n");
        log_rapl_info(rapl);
        printf("\n\n GET ENERGY %lf\n\n", rapl_get_energy(rapl));
    }

    return 0;
}
