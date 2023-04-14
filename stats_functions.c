#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <utmp.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <sys/utsname.h>
#include  <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "a3.h"
#define MAX_LEN 1024




// Function to retrieve CPU statistics
void getCpu(int pipefd_cpu[2]){
    // Open "/proc/stat" file
    FILE *fp = fopen("/proc/stat", "r");
    char cpu_n[255];
    // Declare a structure to store CPU statistics
    struct cpu_stat *cpu;
    // Read CPU statistics from file
    fscanf(fp, "%s %ld %ld %ld %ld %ld %ld %ld", cpu_n, &(cpu->cpu_user), &(cpu->cpu_nice), 
        &(cpu->cpu_system), &(cpu->cpu_idle), &(cpu->cpu_iowait), &(cpu->cpu_irq),
        &(cpu->cpu_softirq));
    // Close the file
    fclose(fp);
    // Write the CPU statistics to the pipe
    int cpu_write = write(pipefd_cpu[1], cpu, sizeof(struct cpu_stat));
    if (cpu_write == -1){
        fprintf(stderr, "Unable to write to pipe (%s)\n", strerror(errno));
        exit(1);
    }
	return;
}


// Function to calculate the load on the CPU
double calculate_load(struct cpu_stat *cur, long int* prev_idle, long int* prev_load){
    
    // Calculates total usage of cpu
    long int cpu_total = cur -> cpu_user + cur -> cpu_nice + cur -> cpu_system + cur -> cpu_iowait + cur -> cpu_irq + cur -> cpu_softirq;

    // Cpu value calculations (Same as assignment)
    long int total_prev = *prev_load + *prev_idle; // Cpu total;
    long int total_cur = cur -> cpu_idle + cpu_total;
    double totald = (double) total_cur - (double) total_prev;
    double idled = (double) cur -> cpu_idle - (double) *prev_idle;
    double load = fabs((100 * (totald - idled) / (totald) + 1));

    *prev_idle = cur -> cpu_idle;
    *prev_load = cpu_total;
    return load;
}


// Function to display the current users on the system
void userPrint(int pipefd[2]){

    // Declare a structure to store information about a user process
    struct utmp *ut;
    // Open the user database
    setutent();
    // Loop through the entries in the database
	
    while ((ut = getutent())) {
        // If the entry represents a user process
        char store[1024];
        snprintf(store, 1024, "%s\t %s (%s)\n", ut->ut_user, ut->ut_line, ut->ut_host);
        write(pipefd[1], store, strlen(store));
        }
    
    // Close the user accounting database
    endutent();  

    close(pipefd[1]);
}

//Function to display the memory usage of the system.
void cpuPrint(char graph_arr[][1024], int tdelay, int samples, bool graph, int i, long int* prev_idle, long int* prev_cpu_load, struct cpu_stat *cpu_stat) {
        double prev_load = calculate_load(cpu_stat, prev_idle, prev_cpu_load);
        if (i != -1) {
            int core = sysconf(_SC_NPROCESSORS_ONLN);
            printf("---------------------------------------\n");
            printf("Number of cores: %d\n", core);
            printf(" total cpu use =  %.2f%%\n", prev_load);
            if (graph) {
                char str[100];
                char sep[100];
                memset(str, '\0', sizeof(str));
                if (prev_load < 0) {prev_load = 0;}
                if (prev_load > 100) {prev_load = 100;}
                if (prev_load <= 0.01) {sprintf(str, "||  %.2f%%\n", prev_load);}
                else {
                    int barCount = 0;
                    if (prev_load > 0.01 && prev_load <= 1.00) {barCount = (int)(prev_load * 10); }
                    else if (prev_load > 1.00 && prev_load <= 10.00) {barCount = prev_load + 10;}
                    else if (prev_load > 10.00 && prev_load <= 100.00) {barCount = prev_load / 10 + 20;}
                    memset(str, '|', barCount);
                    sprintf(sep, "|| %.2f%%\n", prev_load);
                    strcat(str, sep);
                }
                //copy the string to the array
                strcpy(graph_arr[i], str);
                //print the array samples times as the terminal is cleared after every iteration
                for (int j = 0; j < samples; j++) {
                    printf("\t%s", graph_arr[j]);
                }
                for (int k = samples - 1; k > i; k--) {
                    printf("\n");
                }
            }
        }
}

void getMemory(int pipedf_mem[2]){
    //printf("getMemory start\n");
    struct mem_stat* mem_stat;
	// Function stores memory output in struct
	struct sysinfo sys;
    sysinfo(&sys);
    double virt_change = 0;
    long long phys_mem = sys.totalram * sys.mem_unit;
    long long phys_used = (sys.totalram - sys.freeram) * sys.mem_unit;
    long long virt_mem = sys.totalram * sys.mem_unit + sys.totalswap * sys.mem_unit;
    long long virt_used = (sys.totalram - sys.freeram) * sys.mem_unit + (sys.totalswap - sys.freeswap) * sys.mem_unit;
    //convert to GB
    mem_stat -> used_memory = (double)phys_used / (1024 * 1024 * 1024);
    mem_stat -> total_memory = (double)phys_mem / (1024 * 1024 * 1024);
    mem_stat -> used_virtual = (double)virt_used / (1024 * 1024 * 1024);
    mem_stat -> total_virtual = (double)virt_mem / (1024 * 1024 * 1024);

    int mem_write = write(pipedf_mem[1], &mem_stat, sizeof(mem_stat));
    if (mem_write == -1){
        kill(getpid(), SIGTERM);
        kill(getppid(), SIGTERM);
    }
    //printf("getMemory end\n");
    return;
}


void memoryPrint(char memory_arr[][1024], int samples, bool graph, int i, double* prev_virt, struct mem_stat *mem_stat){
    printf("memoryPrint start\n");
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
    // Declare a structure to store memory usage information
    double fin_phys_used = mem_stat -> used_memory;
    double fin_phys_mem = mem_stat -> total_memory;
    double fin_virt_used = mem_stat -> used_virtual;
    double fin_virt_mem = mem_stat -> total_virtual;
   // printf("checkers\n");
    double virt_change = (double)(fin_virt_used - *prev_virt);
    //printf("check\n");
    // virt_change = virt_change - (int)virt_change * 100;
    char str[200];
    char sep[200];
    char mem[200];
    char symbols[200];
    //Ensure all arrays are empty
    //printf("check0\n");

    memset(str, '\0', sizeof(str));
    memset(sep, '\0', sizeof(sep));
    memset(symbols, '\0', sizeof(symbols));

   // printf("check1\n");

    if(graph) {sprintf(mem, "%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", fin_phys_used, fin_phys_mem, fin_virt_used, fin_virt_mem);}
    else {sprintf(mem, "%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", fin_phys_used, fin_phys_mem, fin_virt_used, fin_virt_mem);}
    
    if (graph)
    {
        //skip first iteration
        if(i !=0){
            if (virt_change == 0.00){sprintf(sep, "    |o %.2f (%.2f)\n", virt_change, fin_virt_used);}
            //if virt_change is positive
            else if (virt_change > 0) {
                strcat(str, "    |");
                //for each 0.01 change, add a symbol
                memset(symbols, '#',(int)((double)virt_change * 100));
                strcat(str, symbols);
                sprintf(sep, "* %.2f  (%.2f)\n", virt_change, fin_virt_used);
                
            } 
            //if virt_change is negative
            else {
                strcat(str, "    |");
                //for each 0.01 change, add a symbol
                memset(symbols, ':', (int)abs(((double)virt_change* 100)));
                strcat(str, symbols);
                sprintf(sep, "@ %.2f  (%.2f)\n", fabs(virt_change), fin_virt_used);
            }
            strcat(str, sep);
            strcat(mem, str);
        }
        else{
            //first iteration
            sprintf(sep, "    |o 0.00 (%.2f)\n", fin_virt_used);
            strcat(mem, sep);
        }
        
    }
  //  printf("check2\n");
    strcpy(memory_arr[i], mem);
   // printf("check3\n");
    //print memory array
    for (int j = 0; j < samples; j++)
    {
        printf("%s", memory_arr[j]);
    }
    for (int k = samples-1; k > i; k--)
    {
        printf("\n");
    }  
 //   printf("memoryPrint end\n");  
}


// Function to display the Number of samples and the time delay in seconds, and the memory usage
void starter(int samples, int tdelay){
    printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
    // Declare a structure to store memory usage information
    struct rusage mem_use;
    struct rusage *ptr = &mem_use;
    // Use the getrusage() function to retrieve information about the memory usage of the calling process
    if(getrusage(RUSAGE_SELF, ptr)==0){
        printf(" Memory usage: %ld kilobytes\n", ptr->ru_maxrss);
    }
    else{
        fprintf(stderr, "Unable to write to pipe (%s)\n", strerror(errno));
        exit(1);
    }
}

// Function to display the system information
void ender(){
    printf("---------------------------------------\n");
    printf("### System Information ### \n");
    // Declare a structure to store system information
    struct utsname utsname;
    // Use the uname() function to fill the structure with information about the system
    int m = uname(&utsname);
    if(m == -1){
        fprintf(stderr, "Unable to write to pipe (%s)\n", strerror(errno));
        exit(1);
    }
    // Display the relevant system information
    printf(" System Name = %s\n", utsname.sysname);
    printf(" Machine Name = %s\n", utsname.nodename);
    printf(" Version = %s\n", utsname.version);
    printf(" Release = %s\n", utsname.release);
    printf(" Architecture = %s\n", utsname.machine);
    printf("---------------------------------------\n");
}