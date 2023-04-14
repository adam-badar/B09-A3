#ifndef __A3_header
#define __A3_header

#define MAX_LEN 1024

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


// struct to store memory statistics
struct mem_stat {
    double total_mem;
    double used_mem;
    double total_virt;
    double used_virt;
};

void getCpu(int pipefd_cpu[2]);

double calculate_load(struct cpu_stat *cur, long int* prev_idle, long int* prev_load);

void starter(int samples, int tdelay);

void ender();

void starter(int samples, int tdelay);

void userPrint(int pipefd[2]);

void cpuPrint(char graph_arr[][1024], int samples, bool graph, int i, long int* prev_idle, long int* prev_load, struct cpu_stat *cpu_stat);

void getMemory(int pipedf_mem[2]);

void memoryPrint(char memory_arr[1024][1024], int samples, bool graph, int i, double* prev_virt, struct mem_stat *mem_stat);

#endif