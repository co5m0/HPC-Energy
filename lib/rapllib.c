#include "rapllib.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

// typedef struct rapl_info_s {
//     unsigned int msr_rapl_units;
//     unsigned int msr_pp0_energy_status;
//     unsigned int msr_pkg_energy_status;
//     int package_map[MAX_PACKAGES];
//     int total_cores;
//     int total_packages;
//     int cpu_model;
//     double power_units, time_units;
//     double cpu_energy_units[MAX_PACKAGES], dram_energy_units[MAX_PACKAGES];
//     double package_before[MAX_PACKAGES], package_after[MAX_PACKAGES];
//     double pp0_before[MAX_PACKAGES], pp0_after[MAX_PACKAGES];
//     double pp1_before[MAX_PACKAGES], pp1_after[MAX_PACKAGES];
//     double dram_before[MAX_PACKAGES], dram_after[MAX_PACKAGES];
//     double psys_before[MAX_PACKAGES], psys_after[MAX_PACKAGES];
//     double thermal_spec_power, minimum_power, maximum_power, time_window;
//     int dram_avail, pp0_avail, pp1_avail, psys_avail;
//     int different_units;
// } rapl_info_s;

Rapl_info new_rapl_info() {
    Rapl_info rapl = malloc(sizeof(rapl_info_s));
    rapl->msr_rapl_units = 0;
    rapl->msr_pp0_energy_status = 0;
    rapl->msr_pkg_energy_status = 0;
    rapl->total_cores = 0;
    rapl->total_packages = 0;
    rapl->cpu_model = 0;
    rapl->power_units = 0;
    rapl->time_units = 0;
    rapl->thermal_spec_power = 0;
    rapl->minimum_power = 0;
    rapl->maximum_power = 0;
    rapl->time_window = 0;
    rapl->dram_avail = 0;
    rapl->pp0_avail = 0;
    rapl->pp1_avail = 0;
    rapl->psys_avail = 0;
    rapl->different_units = 0;

    return rapl;
}

void log_rapl_info(Rapl_info rapl) {
    int j, i;
    for (j = 0; j < rapl->total_packages; j++) {
        printf("\t\tCPU Energy units = %.8fJ\n", rapl->cpu_energy_units[j]);
        printf("\t\tDRAM Energy units = %.8fJ\n", rapl->dram_energy_units[j]);
    }
    printf("\t\tPower units = %.3fW\n", rapl->power_units);
    printf("\t\tTime units = %.8fs\n", rapl->time_units);
    printf("\t\tDetected %d cores in %d packages\n\n",
           rapl->total_cores, rapl->total_packages);
    printf("\t\tCPU model %d\n", rapl->cpu_model);
    printf("\t\tPackage thermal spec: %.3fW\n", rapl->thermal_spec_power);
    printf("\t\tPackage minimum power: %.3fW\n", rapl->minimum_power);
    printf("\t\tPackage maximum power: %.3fW\n", rapl->maximum_power);
    printf("\t\tPackage maximum time window: %.6fs\n", rapl->time_window);
    // printf("\t\tPackage power limit #2: %.3fW for %.6fs\n",
    //                pkg_power_limit_2, pkg_time_window_2);
    for (j = 0; j < rapl->total_packages; j++) {
        printf("\tPackage %d\n", j);
        for (i = 0; i < NUM_RAPL_DOMAINS; i++) {
            if (rapl->sysfs_valid[j][i]) {
                printf("\t\t%s\t: %lfJ\n", rapl->sysfs_event_names[j][i],
                       ((double)rapl->sysfs_after[j][i] - (double)rapl->sysfs_before[j][i]) / 1000000.0);
            }
        }
    }
    printf("\n");
}

static int open_msr(int core) {
    char msr_filename[BUFSIZ];
    int fd;

    sprintf(msr_filename, "/dev/cpu/%d/msr", core);
    fd = open(msr_filename, O_RDONLY);
    if (fd < 0) {
        if (errno == ENXIO) {
            fprintf(stderr, "rdmsr: No CPU %d\n", core);
            exit(2);
        } else if (errno == EIO) {
            fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n",
                    core);
            exit(3);
        } else {
            perror("rdmsr:open");
            fprintf(stderr, "Trying to open %s\n", msr_filename);
            exit(127);
        }
    }

    return fd;
}

static long long read_msr(int fd, unsigned int which) {
    uint64_t data;

    if (pread(fd, &data, sizeof data, which) != sizeof data) {
        perror("rdmsr:pread");
        fprintf(stderr, "Error reading MSR %x\n", which);
        exit(127);
    }

    return (long long)data;
}

int detect_cpu(Rapl_info rapl) {
    FILE *fff;

    int vendor = -1, family, model = -1;
    char buffer[BUFSIZ], *result;
    char vendor_string[BUFSIZ];

    fff = fopen("/proc/cpuinfo", "r");
    if (fff == NULL) return -1;

    while (1) {
        result = fgets(buffer, BUFSIZ, fff);
        if (result == NULL) break;

        if (!strncmp(result, "vendor_id", 8)) {
            sscanf(result, "%*s%*s%s", vendor_string);

            if (!strncmp(vendor_string, "GenuineIntel", 12)) {
                vendor = CPU_VENDOR_INTEL;
            }
            if (!strncmp(vendor_string, "AuthenticAMD", 12)) {
                vendor = CPU_VENDOR_AMD;
            }
        }

        if (!strncmp(result, "cpu family", 10)) {
            sscanf(result, "%*s%*s%*s%d", &family);
        }

        if (!strncmp(result, "model", 5)) {
            sscanf(result, "%*s%*s%d", &model);
        }
    }

    if (vendor == CPU_VENDOR_INTEL) {
        if (family != 6) {
            printf("Wrong CPU family %d\n", family);
            return -1;
        }

        rapl->msr_rapl_units = MSR_INTEL_RAPL_POWER_UNIT;
        rapl->msr_pkg_energy_status = MSR_INTEL_PKG_ENERGY_STATUS;
        rapl->msr_pp0_energy_status = MSR_INTEL_PP0_ENERGY_STATUS;

        printf("Found ");

        switch (model) {
            case CPU_SANDYBRIDGE:
                printf("Sandybridge");
                break;
            case CPU_SANDYBRIDGE_EP:
                printf("Sandybridge-EP");
                break;
            case CPU_IVYBRIDGE:
                printf("Ivybridge");
                break;
            case CPU_IVYBRIDGE_EP:
                printf("Ivybridge-EP");
                break;
            case CPU_HASWELL:
            case CPU_HASWELL_ULT:
            case CPU_HASWELL_GT3E:
                printf("Haswell");
                break;
            case CPU_HASWELL_EP:
                printf("Haswell-EP");
                break;
            case CPU_BROADWELL:
            case CPU_BROADWELL_GT3E:
                printf("Broadwell");
                break;
            case CPU_BROADWELL_EP:
                printf("Broadwell-EP");
                break;
            case CPU_SKYLAKE:
            case CPU_SKYLAKE_HS:
                printf("Skylake");
                break;
            case CPU_SKYLAKE_X:
                printf("Skylake-X");
                break;
            case CPU_KABYLAKE:
            case CPU_KABYLAKE_MOBILE:
                printf("Kaby Lake");
                break;
            case CPU_KNIGHTS_LANDING:
                printf("Knight's Landing");
                break;
            case CPU_KNIGHTS_MILL:
                printf("Knight's Mill");
                break;
            case CPU_ATOM_GOLDMONT:
            case CPU_ATOM_GEMINI_LAKE:
            case CPU_ATOM_DENVERTON:
                printf("Atom");
                break;
            default:
                printf("Unsupported model %d\n", model);
                model = -1;
                break;
        }
    }

    if (vendor == CPU_VENDOR_AMD) {
        rapl->msr_rapl_units = MSR_AMD_RAPL_POWER_UNIT;
        rapl->msr_pkg_energy_status = MSR_AMD_PKG_ENERGY_STATUS;
        rapl->msr_pp0_energy_status = MSR_AMD_PP0_ENERGY_STATUS;

        if (family != 23) {
            printf("Wrong CPU family %d\n", family);
            return -1;
        }
        model = CPU_AMD_FAM17H;
    }

    printf(" Processor type\n");

    rapl->cpu_model = model;

    return fclose(fff);
}

int detect_packages(Rapl_info rapl) {
    char filename[BUFSIZ];
    FILE *fff;
    int package;
    int i;

    for (i = 0; i < MAX_PACKAGES; i++) rapl->package_map[i] = -1;

    printf("\t");
    for (i = 0; i < MAX_CPUS; i++) {
        sprintf(filename, "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", i);
        fff = fopen(filename, "r");
        if (fff == NULL) break;
        fscanf(fff, "%d", &package);
        printf("%d (%d)", i, package);
        if (i % 8 == 7)
            printf("\n\t");
        else
            printf(", ");
        fclose(fff);

        if (rapl->package_map[package] == -1) {
            rapl->total_packages++;
            rapl->package_map[package] = i;
        }
    }

    printf("\n");

    rapl->total_cores = i;

    printf("\tDetected %d cores in %d packages\n\n",
           rapl->total_cores, rapl->total_packages);

    return 0;
}

// dive this function in:
// - start -> save in rapl_info
// - stop -> save in rapl_info
// thene save the differenze in rapl_info
int rapl_msr(int core, Rapl_info rapl) {
    int fd;
    long long result;
    // double power_units, time_units;
    // double cpu_energy_units[MAX_PACKAGES], dram_energy_units[MAX_PACKAGES];
    // double package_before[MAX_PACKAGES], package_after[MAX_PACKAGES];
    // double pp0_before[MAX_PACKAGES], pp0_after[MAX_PACKAGES];
    // double pp1_before[MAX_PACKAGES], pp1_after[MAX_PACKAGES];
    // double dram_before[MAX_PACKAGES], dram_after[MAX_PACKAGES];
    // double psys_before[MAX_PACKAGES], psys_after[MAX_PACKAGES];
    // double thermal_spec_power, minimum_power, maximum_power, time_window;
    int j;

    // int dram_avail = 0, pp0_avail = 0, pp1_avail = 0, psys_avail = 0;
    // int different_units = 0;

    printf("\nTrying /dev/msr interface to gather results\n\n");

    if (rapl->cpu_model < 0) {
        printf("\tUnsupported CPU model %d\n", rapl->cpu_model);
        return -1;
    }

    // set availabilities for your CPU
    switch (rapl->cpu_model) {
        case CPU_SANDYBRIDGE_EP:
        case CPU_IVYBRIDGE_EP:
            rapl->pp0_avail = 1;
            rapl->pp1_avail = 0;
            rapl->dram_avail = 1;
            rapl->different_units = 0;
            rapl->psys_avail = 0;
            break;

        case CPU_HASWELL_EP:
        case CPU_BROADWELL_EP:
        case CPU_SKYLAKE_X:
            rapl->pp0_avail = 1;
            rapl->pp1_avail = 0;
            rapl->dram_avail = 1;
            rapl->different_units = 1;
            rapl->psys_avail = 0;
            break;

        case CPU_KNIGHTS_LANDING:
        case CPU_KNIGHTS_MILL:
            rapl->pp0_avail = 0;
            rapl->pp1_avail = 0;
            rapl->dram_avail = 1;
            rapl->different_units = 1;
            rapl->psys_avail = 0;
            break;

        case CPU_SANDYBRIDGE:
        case CPU_IVYBRIDGE:
            rapl->pp0_avail = 1;
            rapl->pp1_avail = 1;
            rapl->dram_avail = 0;
            rapl->different_units = 0;
            rapl->psys_avail = 0;
            break;

        case CPU_HASWELL:
        case CPU_HASWELL_ULT:
        case CPU_HASWELL_GT3E:
        case CPU_BROADWELL:
        case CPU_BROADWELL_GT3E:
        case CPU_ATOM_GOLDMONT:
        case CPU_ATOM_GEMINI_LAKE:
        case CPU_ATOM_DENVERTON:
            rapl->pp0_avail = 1;
            rapl->pp1_avail = 1;
            rapl->dram_avail = 1;
            rapl->different_units = 0;
            rapl->psys_avail = 0;
            break;

        case CPU_SKYLAKE:
        case CPU_SKYLAKE_HS:
        case CPU_KABYLAKE:
        case CPU_KABYLAKE_MOBILE:
            rapl->pp0_avail = 1;
            rapl->pp1_avail = 1;
            rapl->dram_avail = 1;
            rapl->different_units = 0;
            rapl->psys_avail = 1;
            break;

        case CPU_AMD_FAM17H:
            rapl->pp0_avail = 1;  // maybe
            rapl->pp1_avail = 0;
            rapl->dram_avail = 0;
            rapl->different_units = 0;
            rapl->psys_avail = 0;
            break;
    }

    // Listing paramaters for package
    for (j = 0; j < rapl->total_packages; j++) {
        printf("\tListing paramaters for package #%d\n", j);

        fd = open_msr(rapl->package_map[j]);

        /* Calculate the units used */
        result = read_msr(fd, rapl->msr_rapl_units);

        rapl->power_units = pow(0.5, (double)(result & 0xf));
        rapl->cpu_energy_units[j] = pow(0.5, (double)((result >> 8) & 0x1f));
        rapl->time_units = pow(0.5, (double)((result >> 16) & 0xf));

        /* On Haswell EP and Knights Landing */
        /* The DRAM units differ from the CPU ones */
        if (rapl->different_units) {
            rapl->dram_energy_units[j] = pow(0.5, (double)16);
            printf("DRAM: Using %lf instead of %lf\n",
                   rapl->dram_energy_units[j], rapl->cpu_energy_units[j]);
        } else {
            rapl->dram_energy_units[j] = rapl->cpu_energy_units[j];
        }

        printf("\t\tPower units = %.3fW\n", rapl->power_units);
        printf("\t\tCPU Energy units = %.8fJ\n", rapl->cpu_energy_units[j]);
        printf("\t\tDRAM Energy units = %.8fJ\n", rapl->dram_energy_units[j]);
        printf("\t\tTime units = %.8fs\n", rapl->time_units);
        printf("\n");

        if (rapl->cpu_model != CPU_AMD_FAM17H) {
            /* Show package power info */
            result = read_msr(fd, MSR_PKG_POWER_INFO);
            rapl->thermal_spec_power = rapl->power_units * (double)(result & 0x7fff);
            printf("\t\tPackage thermal spec: %.3fW\n", rapl->thermal_spec_power);
            rapl->minimum_power = rapl->power_units * (double)((result >> 16) & 0x7fff);
            printf("\t\tPackage minimum power: %.3fW\n", rapl->minimum_power);
            rapl->maximum_power = rapl->power_units * (double)((result >> 32) & 0x7fff);
            printf("\t\tPackage maximum power: %.3fW\n", rapl->maximum_power);
            rapl->time_window = rapl->time_units * (double)((result >> 48) & 0x7fff);
            printf("\t\tPackage maximum time window: %.6fs\n", rapl->time_window);

            /* Show package power limit */
            result = read_msr(fd, MSR_PKG_RAPL_POWER_LIMIT);
            printf("\t\tPackage power limits are %s\n", (result >> 63) ? "locked" : "unlocked");
            double pkg_power_limit_1 = rapl->power_units * (double)((result >> 0) & 0x7FFF);
            double pkg_time_window_1 = rapl->time_units * (double)((result >> 17) & 0x007F);
            printf("\t\tPackage power limit #1: %.3fW for %.6fs (%s, %s)\n",
                   pkg_power_limit_1, pkg_time_window_1,
                   (result & (1LL << 15)) ? "enabled" : "disabled",
                   (result & (1LL << 16)) ? "clamped" : "not_clamped");
            double pkg_power_limit_2 = rapl->power_units * (double)((result >> 32) & 0x7FFF);
            double pkg_time_window_2 = rapl->time_units * (double)((result >> 49) & 0x007F);
            printf("\t\tPackage power limit #2: %.3fW for %.6fs (%s, %s)\n",
                   pkg_power_limit_2, pkg_time_window_2,
                   (result & (1LL << 47)) ? "enabled" : "disabled",
                   (result & (1LL << 48)) ? "clamped" : "not_clamped");
        }

        /* only available on *Bridge-EP */
        if ((rapl->cpu_model == CPU_SANDYBRIDGE_EP) || (rapl->cpu_model == CPU_IVYBRIDGE_EP)) {
            result = read_msr(fd, MSR_PKG_PERF_STATUS);
            double acc_pkg_throttled_time = (double)result * rapl->time_units;
            printf("\tAccumulated Package Throttled Time : %.6fs\n",
                   acc_pkg_throttled_time);
        }

        /* only available on *Bridge-EP */
        if ((rapl->cpu_model == CPU_SANDYBRIDGE_EP) || (rapl->cpu_model == CPU_IVYBRIDGE_EP)) {
            result = read_msr(fd, MSR_PP0_PERF_STATUS);
            double acc_pp0_throttled_time = (double)result * rapl->time_units;
            printf(
                "\tPowerPlane0 (core) Accumulated Throttled Time "
                ": %.6fs\n",
                acc_pp0_throttled_time);

            result = read_msr(fd, MSR_PP0_POLICY);
            int pp0_policy = (int)result & 0x001f;
            printf("\tPowerPlane0 (core) for core %d policy: %d\n", core, pp0_policy);
        }

        if (rapl->pp1_avail) {
            result = read_msr(fd, MSR_PP1_POLICY);
            int pp1_policy = (int)result & 0x001f;
            printf("\tPowerPlane1 (on-core GPU if avail) %d policy: %d\n",
                   core, pp1_policy);
        }
        close(fd);
    }
    printf("\n");

    // for (j = 0; j < rapl->total_packages; j++) {
    //     fd = open_msr(rapl->package_map[j]);

    //     /* Package Energy */
    //     result = read_msr(fd, rapl->msr_pkg_energy_status);
    //     rapl->package_before[j] = (double)result * rapl->cpu_energy_units[j];

    //     /* PP0 energy */
    //     /* Not available on Knights* */
    //     /* Always returns zero on Haswell-EP? */
    //     if (rapl->pp0_avail) {
    //         result = read_msr(fd, rapl->msr_pp0_energy_status);
    //         rapl->pp0_before[j] = (double)result * rapl->cpu_energy_units[j];
    //     }

    //     /* PP1 energy */
    //     /* not available on *Bridge-EP */
    //     if (rapl->pp1_avail) {
    //         result = read_msr(fd, MSR_PP1_ENERGY_STATUS);
    //         rapl->pp1_before[j] = (double)result * rapl->cpu_energy_units[j];
    //     }

    //     /* Updated documentation (but not the Vol3B) says Haswell and	*/
    //     /* Broadwell have DRAM support too				*/
    //     if (rapl->dram_avail) {
    //         result = read_msr(fd, MSR_DRAM_ENERGY_STATUS);
    //         rapl->dram_before[j] = (double)result * rapl->dram_energy_units[j];
    //     }

    //     /* Skylake and newer for Psys				*/
    //     if (rapl->psys_avail) {
    //         result = read_msr(fd, MSR_PLATFORM_ENERGY_STATUS);
    //         rapl->psys_before[j] = (double)result * rapl->cpu_energy_units[j];
    //     }

    //     close(fd);
    // }

    // printf("\n\tSleeping 1 second\n\n");
    // sleep(1);

    // for (j = 0; j < rapl->total_packages; j++) {
    //     fd = open_msr(rapl->package_map[j]);

    //     printf("\tPackage %d:\n", j);

    //     result = read_msr(fd, rapl->msr_pkg_energy_status);
    //     rapl->package_after[j] = (double)result * rapl->cpu_energy_units[j];
    //     printf("\t\tPackage energy: %.6fJ\n",
    //            rapl->package_after[j] - rapl->package_before[j]);

    //     result = read_msr(fd, rapl->msr_pp0_energy_status);
    //     rapl->pp0_after[j] = (double)result * rapl->cpu_energy_units[j];
    //     printf("\t\tPowerPlane0 (cores): %.6fJ\n",
    //            rapl->pp0_after[j] - rapl->pp0_before[j]);

    //     /* not available on SandyBridge-EP */
    //     if (rapl->pp1_avail) {
    //         result = read_msr(fd, MSR_PP1_ENERGY_STATUS);
    //         rapl->pp1_after[j] = (double)result * rapl->cpu_energy_units[j];
    //         printf("\t\tPowerPlane1 (on-core GPU if avail): %.6f J\n",
    //                rapl->pp1_after[j] - rapl->pp1_before[j]);
    //     }

    //     if (rapl->dram_avail) {
    //         result = read_msr(fd, MSR_DRAM_ENERGY_STATUS);
    //         rapl->dram_after[j] = (double)result * rapl->dram_energy_units[j];
    //         printf("\t\tDRAM: %.6fJ\n",
    //                rapl->dram_after[j] - rapl->dram_before[j]);
    //     }

    //     if (rapl->psys_avail) {
    //         result = read_msr(fd, MSR_PLATFORM_ENERGY_STATUS);
    //         rapl->psys_after[j] = (double)result * rapl->cpu_energy_units[j];
    //         printf("\t\tPSYS: %.6fJ\n",
    //                rapl->psys_after[j] - rapl->psys_before[j]);
    //     }

    //     close(fd);
    // }
    printf("\n");
    printf("Note: the energy measurements can overflow in 60s or so\n");
    printf("      so try to sample the counters more often than that.\n\n");

    return 0;
}

int rapl_msr_start(Rapl_info rapl) {
    int j;
    int fd;
    long long result;

    for (j = 0; j < rapl->total_packages; j++) {
        fd = open_msr(rapl->package_map[j]);

        /* Package Energy */
        result = read_msr(fd, rapl->msr_pkg_energy_status);
        rapl->package_before[j] = (double)result * rapl->cpu_energy_units[j];

        /* PP0 energy */
        /* Not available on Knights* */
        /* Always returns zero on Haswell-EP? */
        if (rapl->pp0_avail) {
            result = read_msr(fd, rapl->msr_pp0_energy_status);
            rapl->pp0_before[j] = (double)result * rapl->cpu_energy_units[j];
        }

        /* PP1 energy */
        /* not available on *Bridge-EP */
        if (rapl->pp1_avail) {
            result = read_msr(fd, MSR_PP1_ENERGY_STATUS);
            rapl->pp1_before[j] = (double)result * rapl->cpu_energy_units[j];
        }

        /* Updated documentation (but not the Vol3B) says Haswell and	*/
        /* Broadwell have DRAM support too				*/
        if (rapl->dram_avail) {
            result = read_msr(fd, MSR_DRAM_ENERGY_STATUS);
            rapl->dram_before[j] = (double)result * rapl->dram_energy_units[j];
        }

        /* Skylake and newer for Psys				*/
        if (rapl->psys_avail) {
            result = read_msr(fd, MSR_PLATFORM_ENERGY_STATUS);
            rapl->psys_before[j] = (double)result * rapl->cpu_energy_units[j];
        }

        close(fd);
    }

    return 0;
}

int rapl_msr_stop(Rapl_info rapl) {
    int j;
    int fd;
    long long result;

    for (j = 0; j < rapl->total_packages; j++) {
        fd = open_msr(rapl->package_map[j]);

        printf("\tPackage %d:\n", j);

        result = read_msr(fd, rapl->msr_pkg_energy_status);
        rapl->package_after[j] = (double)result * rapl->cpu_energy_units[j];
        printf("\t\tPackage energy: %.6fJ\n",
               rapl->package_after[j] - rapl->package_before[j]);

        result = read_msr(fd, rapl->msr_pp0_energy_status);
        rapl->pp0_after[j] = (double)result * rapl->cpu_energy_units[j];
        printf("\t\tPowerPlane0 (cores): %.6fJ\n",
               rapl->pp0_after[j] - rapl->pp0_before[j]);

        /* not available on SandyBridge-EP */
        if (rapl->pp1_avail) {
            result = read_msr(fd, MSR_PP1_ENERGY_STATUS);
            rapl->pp1_after[j] = (double)result * rapl->cpu_energy_units[j];
            printf("\t\tPowerPlane1 (on-core GPU if avail): %.6f J\n",
                   rapl->pp1_after[j] - rapl->pp1_before[j]);
        }

        if (rapl->dram_avail) {
            result = read_msr(fd, MSR_DRAM_ENERGY_STATUS);
            rapl->dram_after[j] = (double)result * rapl->dram_energy_units[j];
            printf("\t\tDRAM: %.6fJ\n",
                   rapl->dram_after[j] - rapl->dram_before[j]);
        }

        if (rapl->psys_avail) {
            result = read_msr(fd, MSR_PLATFORM_ENERGY_STATUS);
            rapl->psys_after[j] = (double)result * rapl->cpu_energy_units[j];
            printf("\t\tPSYS: %.6fJ\n",
                   rapl->psys_after[j] - rapl->psys_before[j]);
        }

        close(fd);
    }

    return 0;
}

int rapl_sysfs(Rapl_info rapl) {
    // char event_names[MAX_PACKAGES][NUM_RAPL_DOMAINS][256];
    // char filenames[MAX_PACKAGES][NUM_RAPL_DOMAINS][256];
    char basename[MAX_PACKAGES][256];
    char tempfile[256];

    // long long before[MAX_PACKAGES][NUM_RAPL_DOMAINS];
    // long long after[MAX_PACKAGES][NUM_RAPL_DOMAINS];

    // int valid[MAX_PACKAGES][NUM_RAPL_DOMAINS];
    int i, j;
    FILE *fff;

    printf("\nTrying sysfs powercap interface to gather results\n\n");

    /* /sys/class/powercap/intel-rapl/intel-rapl:0/ */
    /* name has name */
    /* energy_uj has energy */
    /* subdirectories intel-rapl:0:0 intel-rapl:0:1 intel-rapl:0:2 */

    for (j = 0; j < rapl->total_packages; j++) {
        i = 0;
        sprintf(basename[j], "/sys/class/powercap/intel-rapl/intel-rapl:%d",
                j);
        sprintf(tempfile, "%s/name", basename[j]);
        fff = fopen(tempfile, "r");
        if (fff == NULL) {
            fprintf(stderr, "\tCould not open %s\n", tempfile);
            return -1;
        }
        fscanf(fff, "%s", rapl->sysfs_event_names[j][i]);
        rapl->sysfs_valid[j][i] = 1;
        fclose(fff);
        sprintf(rapl->sysfs_filenames[j][i], "%s/energy_uj", basename[j]);

        /* Handle subdomains */
        for (i = 1; i < NUM_RAPL_DOMAINS; i++) {
            sprintf(tempfile, "%s/intel-rapl:%d:%d/name",
                    basename[j], j, i - 1);
            fff = fopen(tempfile, "r");
            if (fff == NULL) {
                //fprintf(stderr,"\tCould not open %s\n",tempfile);
                rapl->sysfs_valid[j][i] = 0;
                continue;
            }
            rapl->sysfs_valid[j][i] = 1;
            fscanf(fff, "%s", rapl->sysfs_event_names[j][i]);
            fclose(fff);
            sprintf(rapl->sysfs_filenames[j][i], "%s/intel-rapl:%d:%d/energy_uj",
                    basename[j], j, i - 1);
        }
    }

    /* Gather before values */
    // for (j = 0; j < rapl->total_packages; j++) {
    //     for (i = 0; i < NUM_RAPL_DOMAINS; i++) {
    //         if (valid[j][i]) {
    //             fff = fopen(filenames[j][i], "r");
    //             if (fff == NULL) {
    //                 fprintf(stderr, "\tError opening %s!\n", filenames[j][i]);
    //             } else {
    //                 fscanf(fff, "%lld", &before[j][i]);
    //                 fclose(fff);
    //             }
    //         }
    //     }
    // }

    // printf("\tSleeping 1 second\n\n");
    // sleep(1);

    /* Gather after values */
    // for (j = 0; j < rapl->total_packages; j++) {
    //     for (i = 0; i < NUM_RAPL_DOMAINS; i++) {
    //         if (valid[j][i]) {
    //             fff = fopen(filenames[j][i], "r");
    //             if (fff == NULL) {
    //                 fprintf(stderr, "\tError opening %s!\n", filenames[j][i]);
    //             } else {
    //                 fscanf(fff, "%lld", &after[j][i]);
    //                 fclose(fff);
    //             }
    //         }
    //     }
    // }

    // for (j = 0; j < rapl->total_packages; j++) {
    //     printf("\tPackage %d\n", j);
    //     for (i = 0; i < NUM_RAPL_DOMAINS; i++) {
    //         if (valid[j][i]) {
    //             printf("\t\t%s\t: %lfJ\n", event_names[j][i],
    //                    ((double)after[j][i] - (double)before[j][i]) / 1000000.0);
    //         }
    //     }
    // }
    // printf("\n");

    return 0;
}

int rapl_sysfs_start(Rapl_info rapl) {
    int i, j;
    FILE *fff;
    for (j = 0; j < rapl->total_packages; j++) {
        for (i = 0; i < NUM_RAPL_DOMAINS; i++) {
            if (rapl->sysfs_valid[j][i]) {
                fff = fopen(rapl->sysfs_filenames[j][i], "r");
                if (fff == NULL) {
                    fprintf(stderr, "\tError opening %s!\n", rapl->sysfs_filenames[j][i]);
                    exit(0x001);
                } else {
                    fscanf(fff, "%lld", &rapl->sysfs_before[i][j]);
                    // fscanf(fff, "%lld", &before[j][i]);
                    fclose(fff);
                }
            }
        }
    }
    return 0;
}

int rapl_sysfs_stop(Rapl_info rapl) {
    int i, j;
    FILE *fff;
    for (j = 0; j < rapl->total_packages; j++) {
        for (i = 0; i < NUM_RAPL_DOMAINS; i++) {
            if (rapl->sysfs_valid[j][i]) {
                fff = fopen(rapl->sysfs_filenames[j][i], "r");
                if (fff == NULL) {
                    fprintf(stderr, "\tError opening %s!\n", rapl->sysfs_filenames[j][i]);
                    exit(0x001);
                } else {
                    fscanf(fff, "%lld", &rapl->sysfs_after[j][i]);
                    fclose(fff);
                }
            }
        }
    }

    for (j = 0; j < rapl->total_packages; j++) {
        printf("\tPackage %d\n", j);
        for (i = 0; i < NUM_RAPL_DOMAINS; i++) {
            if (rapl->sysfs_valid[j][i]) {
                printf("\t\t%s\t: %lfJ\n", rapl->sysfs_event_names[j][i],
                       ((double)rapl->sysfs_after[j][i] - (double)rapl->sysfs_before[j][i]) / 1000000.0);
            }
        }
    }
    printf("\n");
    return 0;
}