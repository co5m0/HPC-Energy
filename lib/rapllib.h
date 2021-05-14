#define MSR_AMD_RAPL_POWER_UNIT 0xc0010299

#define MSR_AMD_PKG_ENERGY_STATUS 0xc001029B
#define MSR_AMD_PP0_ENERGY_STATUS 0xc001029A

/* Intel support */

#define MSR_INTEL_RAPL_POWER_UNIT 0x606
/*
 * Platform specific RAPL Domains.
 * Note that PP1 RAPL Domain is supported on 062A only
 * And DRAM RAPL Domain is supported on 062D only
 */
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT 0x610
#define MSR_INTEL_PKG_ENERGY_STATUS 0x611
#define MSR_PKG_PERF_STATUS 0x613
#define MSR_PKG_POWER_INFO 0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT 0x638
#define MSR_INTEL_PP0_ENERGY_STATUS 0x639
#define MSR_PP0_POLICY 0x63A
#define MSR_PP0_PERF_STATUS 0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT 0x640
#define MSR_PP1_ENERGY_STATUS 0x641
#define MSR_PP1_POLICY 0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT 0x618
#define MSR_DRAM_ENERGY_STATUS 0x619
#define MSR_DRAM_PERF_STATUS 0x61B
#define MSR_DRAM_POWER_INFO 0x61C

/* PSYS RAPL Domain */
#define MSR_PLATFORM_ENERGY_STATUS 0x64d

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET 0
#define POWER_UNIT_MASK 0x0F

#define ENERGY_UNIT_OFFSET 0x08
#define ENERGY_UNIT_MASK 0x1F00

#define TIME_UNIT_OFFSET 0x10
#define TIME_UNIT_MASK 0xF000

#define CPU_VENDOR_INTEL 1
#define CPU_VENDOR_AMD 2

#define CPU_SANDYBRIDGE 42
#define CPU_SANDYBRIDGE_EP 45
#define CPU_IVYBRIDGE 58
#define CPU_IVYBRIDGE_EP 62
#define CPU_HASWELL 60
#define CPU_HASWELL_ULT 69
#define CPU_HASWELL_GT3E 70
#define CPU_HASWELL_EP 63
#define CPU_BROADWELL 61
#define CPU_BROADWELL_GT3E 71
#define CPU_BROADWELL_EP 79
#define CPU_BROADWELL_DE 86
#define CPU_SKYLAKE 78
#define CPU_SKYLAKE_HS 94
#define CPU_SKYLAKE_X 85
#define CPU_KNIGHTS_LANDING 87
#define CPU_KNIGHTS_MILL 133
#define CPU_KABYLAKE_MOBILE 142
#define CPU_KABYLAKE 158
#define CPU_ATOM_SILVERMONT 55
#define CPU_ATOM_AIRMONT 76
#define CPU_ATOM_MERRIFIELD 74
#define CPU_ATOM_MOOREFIELD 90
#define CPU_ATOM_GOLDMONT 92
#define CPU_ATOM_GEMINI_LAKE 122
#define CPU_ATOM_DENVERTON 95

#define CPU_AMD_FAM17H 0xc000

#define MAX_CPUS 1024
#define MAX_PACKAGES 16

#define NUM_RAPL_DOMAINS 5
typedef struct rapl_info_s {
    unsigned int msr_rapl_units;
    unsigned int msr_pp0_energy_status;
    unsigned int msr_pkg_energy_status;
    int package_map[MAX_PACKAGES];
    int total_cores;
    int total_packages;
    int cpu_model;
    double power_units, time_units;
    double cpu_energy_units[MAX_PACKAGES], dram_energy_units[MAX_PACKAGES];
    double package_before[MAX_PACKAGES], package_after[MAX_PACKAGES];
    double pp0_before[MAX_PACKAGES], pp0_after[MAX_PACKAGES];
    double pp1_before[MAX_PACKAGES], pp1_after[MAX_PACKAGES];
    double dram_before[MAX_PACKAGES], dram_after[MAX_PACKAGES];
    double psys_before[MAX_PACKAGES], psys_after[MAX_PACKAGES];
    char sysfs_filenames[MAX_PACKAGES][NUM_RAPL_DOMAINS][256];
    int sysfs_valid[MAX_PACKAGES][NUM_RAPL_DOMAINS];
    long long sysfs_before[MAX_PACKAGES][NUM_RAPL_DOMAINS];
    long long sysfs_after[MAX_PACKAGES][NUM_RAPL_DOMAINS];
    char sysfs_event_names[MAX_PACKAGES][NUM_RAPL_DOMAINS][256];
    double thermal_spec_power, minimum_power, maximum_power, time_window;
    int dram_avail, pp0_avail, pp1_avail, psys_avail;
    int different_units;
} rapl_info_s;
typedef struct rapl_info_s *Rapl_info;

/**
 * @brief Detect the CPU vendor (Intel, AMD), CPU family, CPU model.
 *
 * @note set: msr_rapl_units, msr_pkg_energy_status, msr_pp0_energy_status, cpu_model
 * 
 * @param rapl Rapl_info structure.
 * @return returns zero if the stream from "/proc/cpuinfo" is successfully closed. On failure, EOF is returned.
 */
int detect_cpu(Rapl_info);

/**
 * Read msr register for a specific core.
 *
 * @param core Core number.
 * @return "/dev/cpu/%d/msr" file descriptor.
 */
// int open_msr(int core);

/**
 * @brief Init Rapl_info structure.
 *
 * @return new Rapl_info structure
 */
Rapl_info new_rapl_info(void);

// long long read_msr(int fd, unsigned int which);

int detect_cpu(Rapl_info rapl);
int detect_packages(Rapl_info rapl);
int rapl_msr(int core, Rapl_info rapl);
int rapl_msr_start(Rapl_info rapl);
int rapl_msr_stop(Rapl_info rapl);
int rapl_sysfs(Rapl_info rapl);
int rapl_sysfs_start(Rapl_info rapl);
int rapl_sysfs_stop(Rapl_info rapl);
void log_rapl_info(Rapl_info rapl);
double rapl_get_energy(Rapl_info rapl);