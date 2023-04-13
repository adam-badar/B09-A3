#ifndef __A3_header
#define __A3_header

#define MAX_LEN 1024

// Struct for holding CPU info after piping 
typedef struct {
    char cpuString[MAX_LEN];
    double prevTime;
    double prevUtil;
    double currCpu;
} CpuStruct;

// Struct for holding memory info after piping
typedef struct {
    char memString[MAX_LEN];
    double prevMem;
} MemStruct;

typedef struct memory(
    double total_virt;
    double used_virt;
    double total_mem;
    double used_mem;
)

//struct to store cpu statistics
struct cpu_stat {
    long int cpu_user;
    long int cpu_nice;
    long int cpu_system;
    long int cpu_idle;
    long int cpu_iowait;
    long int cpu_irq;
    long int cpu_softirq;
};

void get_stats(struct cpu_stat *cpu, int cpunum);

double calculate_load(struct cpu_stat *prev, struct cpu_stat *cur);

void userPrint();

void getCpu(char cpuArray[1024], double *prevTime, double *currCpu, double *prevUtil, bool graphics, int i);

void cpuPrint(char **graph_arr, int tdelay, int samples, double cur_load, int i);

void getMem(char **memory_arr, bool graph, int i, double *prev_virt);

void memPrint(char **memory_arr, int samples, int graph, int i);

void starter(int samples, int tdelay);

void ender();

void CtrlC(int signals);

void CtrlZ(int signals);


#endif