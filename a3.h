#ifndef __A3_header
#define __A3_header



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

double cpuPrint(char **graph_arr, int tdelay, int samples, double prev_load, bool graph, int i);

double memoryPrint(char **memory_arr, int samples, bool graph, int i, double prev_virt);


#endif