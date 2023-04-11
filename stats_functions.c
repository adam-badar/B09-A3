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

#include "a3.h"

// Function to retrieve CPU statistics
void get_stats(struct cpu_stat *cpu, int cpunum)
{
    // Open "/proc/stat" file
    FILE *fp = fopen("/proc/stat", "r");
    char cpu_n[255];
    // Read CPU statistics from file
    fscanf(fp, "%s %ld %ld %ld %ld %ld %ld %ld", cpu_n, &(cpu->cpu_user), &(cpu->cpu_nice), 
        &(cpu->cpu_system), &(cpu->cpu_idle), &(cpu->cpu_iowait), &(cpu->cpu_irq),
        &(cpu->cpu_softirq));
    // Close the file
    fclose(fp);
	return;
}

// Function to calculate the load on the CPU
double calculate_load(struct cpu_stat *prev, struct cpu_stat *cur)
{
    // Calculate the idle time for both previous and current statistics
    int prev_idle = (prev->cpu_idle) + (prev->cpu_iowait);
    int cur_idle = (cur->cpu_idle) + (cur->cpu_iowait);

    // Calculate the load time for both previous and current statistics
    int prev_load = (prev->cpu_user) + (prev->cpu_nice) + (prev->cpu_system) + (prev->cpu_irq) + (prev->cpu_softirq);
    int cur_load = (cur->cpu_user) + (cur->cpu_nice) + (cur->cpu_system) + (cur->cpu_irq) + (cur->cpu_softirq);

    // Calculate the total time for both previous and current statistics
    int total_prev = prev_idle + prev_load;
    int total_cur = cur_idle + cur_load;

    // Calculate the difference in total and idle times
    double total = (double) total_cur - (double) total_prev;
    double idle = (double) cur_idle - (double) prev_idle;

    // Calculate the load percentage
    double load = (1000 * (total - idle) / total + 1) / 10;

    return load;
}

// Function to display the current users on the system
void userPrint(){
    printf("---------------------------------------\n");
    printf("### Sessions/users ###\n");
    // Declare a structure to store information about a user process
    struct utmp *ut;
    // Open the user database
    setutent();
    // Loop through the entries in the database
    while ((ut = getutent())) {
        // If the entry represents a user process
        if (ut->ut_type == USER_PROCESS){
            // Display the username, terminal name, and host name
            printf("%s\t%s\t(%s)\n", ut->ut_user, ut->ut_line, ut->ut_host);
        }
    }
    // Close the user accounting database
    endutent();
  
}

// Function to get the CPU usage of the system
void getCpu()

//Function to print the CPU usage of the system.
double cpuPrint(char **graph_arr, int tdelay, int samples, double prev_load, bool graph, int i) {
        if (i != -1) {
            int core = sysconf(_SC_NPROCESSORS_ONLN);
            printf("---------------------------------------\n");
            printf("Number of cores: %d\n", core);
            printf(" total cpu use =  %.2f%%\n", prev_load);
            if (graph) {
                char str[100];
                char sep[100];
                memset(str, '\0', sizeof(str));
                if(prev_load < 0) {prev_load = 0;}
                if (prev_load <= 0.01) {sprintf(str, "||  %.2f%%\n", prev_load);}
                else {
                    int barCount = 0;
                    if(prev_load>100) {prev_load = 100;}
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
        if(i == samples -1){ return 0;}
        struct cpu_stat prev;
        struct cpu_stat cur;
        //sleep for the time delay
        get_stats(&prev, -1);
        sleep(tdelay);
        get_stats(&cur, -1);
        double load = calculate_load(&prev, &cur);
        return load;
}


double memoryPrint(char **memory_arr, int samples, bool graph, int i, double prev_virt){

        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
        // Declare a structure to store memory usage information
        struct sysinfo sys;
        sysinfo(&sys);
        double virt_change = 0;
        long long phys_mem = sys.totalram * sys.mem_unit;
        long long phys_used = (sys.totalram - sys.freeram) * sys.mem_unit;
        long long virt_mem = sys.totalram * sys.mem_unit + sys.totalswap * sys.mem_unit;
        long long virt_used = (sys.totalram - sys.freeram) * sys.mem_unit + (sys.totalswap - sys.freeswap) * sys.mem_unit;
        //convert to GB
        double fin_phys_used = (double)phys_used / (1024 * 1024 * 1024);
        double fin_phys_mem = (double)phys_mem / (1024 * 1024 * 1024);
        double fin_virt_used = (double)virt_used / (1024 * 1024 * 1024);
        double fin_virt_mem = (double)virt_mem / (1024 * 1024 * 1024);
        virt_change = (double)(fin_virt_used - prev_virt);
        // virt_change = virt_change - (int)virt_change * 100;
        char str[200];
        char sep[200];
        char mem[200];
        char symbols[200];
        //Ensure all arrays are empty
        memset(str, '\0', sizeof(str));
        memset(sep, '\0', sizeof(sep));
        memset(symbols, '\0', sizeof(symbols));
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
        strcpy(memory_arr[i], mem);
        //print memory array
        for (int j = 0; j < samples; j++)
        {
            printf("%s", memory_arr[j]);
        }
        for (int k = samples-1; k > i; k--)
        {
            printf("\n");
        }    
        return fin_virt_used;
}
