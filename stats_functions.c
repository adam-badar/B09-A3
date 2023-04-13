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
#define MAX_LEN 1024


// Function to display the current users on the system
void userPrint(char user_arr[MAX_LEN]){
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
            sprintf(user_arr + strlen(user_arr), "%s\t%s\t(%s)\n", ut->ut_user, ut->ut_line, ut->ut_host);
        }
    }
    // Close the user accounting database
    endutent();
  
}

void getCpu(char cpuArray[1024], double *prevTime, double *currCpu, double *prevUtil, bool graphics, int i) { // PRINTS CPU INFORMATION
    FILE *fp = fopen("/proc/stat", "r");
    long int user, nice, system, idle, iowait, irq, softirq;
    fscanf(fp, "cpu %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq); // access cpu information from file
    fclose(fp);
    double currTime = user + nice + system + idle + iowait + irq + softirq;
    double currUtilization = currTime - idle;
    double totalUtilization = 0;
    if (i > 0) {
        totalUtilization = 100*((currUtilization - * prevUtil) / (currTime - *prevTime));
    }
    if (graphics) {
        strcpy(cpuArray, "\n        ");
        char buffer[1024];
        int numBuffer = (int) fabs(totalUtilization) + 1; // sets the number of characters displayed to be the rounded number of cpu use
        memset(buffer, '|', numBuffer);
        buffer[numBuffer] = '\0';
        sprintf(cpuArray+strlen(cpuArray), "%s %.2lf", buffer, totalUtilization); // copies current string to cpuArray
    }
    *currCpu = totalUtilization; 
    *prevTime = currTime;
    *prevUtil = currUtilization;
}

//Function to print the CPU usage of the system.
void cpuPrint(char graph_arr[][MAX_LEN], int tdelay, int samples, double cur_load, int i) {
          int core = sysconf(_SC_NPROCESSORS_ONLN);
            printf("---------------------------------------\n");
            printf("Number of cores: %d\n", core);
            printf(" total cpu use =  %.2f%%\n", cur_load);
         for (int j = 0; j < samples; j++) {
                    printf("\t%s", graph_arr[j]);
                }
                for (int k = samples - 1; k > i; k--) {
                    printf("\n");
                }
}


void getMem(char memory_arr[][MAX_LEN], bool graph, int i, double *prev_virt){     
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
        virt_change = (double)(fin_virt_used - *prev_virt);
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
            *prev_virt = fin_virt_used;
        }
        strcpy(memory_arr[i], mem);
}

//Function to print the memory usage of the system.
void memPrint(char memory_arr[MAX_LEN], int samples, int graph, int i){
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
            //print memory array
        for (int j = 0; j < samples; j++)
        {
            printf("%s", memory_arr[j]);
        }
        for (int k = samples-1; k > i; k--)
        {
            printf("\n");
        }    
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
}

// Function to display the system information
void ender(){
    printf("---------------------------------------\n");
    printf("### System Information ### \n");
    // Declare a structure to store system information
    struct utsname utsname;
    // Use the uname() function to fill the structure with information about the system
    uname(&utsname);
    // Display the relevant system information
    printf(" System Name = %s\n", utsname.sysname);
    printf(" Machine Name = %s\n", utsname.nodename);
    printf(" Version = %s\n", utsname.version);
    printf(" Release = %s\n", utsname.release);
    printf(" Architecture = %s\n", utsname.machine);
    printf("---------------------------------------\n");
}

//Function to handle ctrl+c
void CtrlC(int signals){
    char command;
    signal(signals, SIG_IGN);
    printf("Enter Y/y to quit, N/n to keep going!\n");
    command = getchar();
    if (command != 'y' || command != 'Y'){
        signal(SIGINT, CtrlC);
    }
    else{
        
        exit(0);
    }
}

//Function to handle ctrl+z
void CtrlZ(int signals){
    //Do nothing
}