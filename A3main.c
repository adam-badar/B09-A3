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
    if (command == 'y' || command == 'Y'){
        exit(0);
    }
    else{
        signal(SIGINT, CtrlC);
    }
}

//Function to handle ctrl+z
void CtrlZ(int signals){
    //Do nothing
}

//Function to print all stats
void finPrint(bool sys, bool user, bool graph, bool sequen, int samples, int tdelay){
    signal(SIGTSTP, CtrlZ);
    signal(SIGINT, CtrlC);
    //declare arrays to store memory and cpu graphical usage
    char **memory_arr = (char **) malloc(samples * sizeof(char *));
    char **graph_arr = (char **) malloc(samples * sizeof(char *));
    for (int i = 0; i < samples; i++) {
        memory_arr[i] = (char *) malloc(200 * sizeof(char));
        graph_arr[i] = (char *) malloc(100 * sizeof(char));
        strcpy(memory_arr[i], "");
        strcpy(graph_arr[i], "");
    }
    double prev_load = 0;
    //prev_load = cpuPrint(graph_arr, tdelay, samples, prev_load, graph, -1);
    double prev_virt = 0;
    struct cpu_stat prev;
    struct cpu_stat cur;
    get_stats(&prev, -1);
    sleep(tdelay);
    get_stats(&cur, -1);
    prev_load = calculate_load(&prev, &cur);
    for (int i = 0; i < samples; i++)
    {
        
        //if sequen is true, print iteration number
        if(sequen){
            printf(">>> iteration %d\n", i);
            starter(samples, tdelay);
            if(sys && !user){
                prev_virt = memoryPrint(memory_arr, samples, graph, i, prev_virt);
                prev_load = cpuPrint(graph_arr, tdelay, samples, prev_load, graph, i);
            }
            else if(user && !sys){
                userPrint();
                sleep(tdelay);
            }
            else if(user && sys){
                prev_virt = memoryPrint(memory_arr, samples, graph, i, prev_virt);
                userPrint();
                prev_load = cpuPrint(graph_arr, tdelay, samples, prev_load, graph, i);
            }
        }
        else{
            printf("\033[H \033[2J \n");
            starter(samples, tdelay);
            if(sys && !user){
                prev_virt = memoryPrint(memory_arr, samples, graph, i, prev_virt);
                prev_load = cpuPrint(graph_arr, tdelay, samples, prev_load, graph, i);
            }
            else if(user && !sys){
                userPrint();
                sleep(tdelay);
            }
            else if(user && sys){
                prev_virt = memoryPrint(memory_arr, samples, graph, i, prev_virt);
                 userPrint();
                prev_load = cpuPrint(graph_arr, tdelay, samples, prev_load, graph, i);      
            }
        }
    }
    ender();
    //free memory
    for (int i = 0; i < samples; i++) {
        free(memory_arr[i]);
        free(graph_arr[i]);
    }
    free(memory_arr);
    free(graph_arr);
}


//Main function where arguments are parsed
int main(int argc, char *argv[]){
    int samples = 10, tdelay = 1;
    //default values
    bool sys = true, user = true, graph = false, sequen = false;
    bool sys_seen = false, user_seen = false;
    bool int_args = false;
    //check for correct number of arguments
    bool any = false;
    for (int i = 0; i < argc; i++)
    {
        if((strcmp(argv[i], "--system") == 0) || (strcmp(argv[i], "-s") == 0)){
            any = true;
            //if user has not been seen, set user to false
            if (user_seen == false) user = false, sys_seen = true;
            //if user has been seen, set user to true
            else{ user = true, sys = true, sys_seen = true;}
        }
        if((strcmp(argv[i], "--user") == 0)|| (strcmp(argv[i], "-u") == 0)){
            any = true;
            //if sys has not been seen, set sys to false
            if (sys_seen == false) sys = false, user_seen = true;
            //if sys has been seen, set sys to true
            else{ sys = true, user = true, user_seen = true;}
        }
        if((strcmp(argv[i], "--graphics") == 0) || (strcmp(argv[i], "-g") == 0)){
            any = true;
            graph = true;
        }
        if((strcmp(argv[i], "--sequential")  == 0)|| (strcmp(argv[i], "-se") == 0)){
            any = true;
            sequen = true;
        }
        //if argument is a number, set samples to that number
        if ((strncmp(argv[i], "--samples=", 10) == 0)){ samples = atoi(argv[i] + 10); any = true;}
        if((strncmp(argv[i], "-sa=", 4) == 0)){ samples = atoi(argv[i] + 4); any = true;} 
        //check for positional arguments
        if ((isdigit(*argv[i])) && (int_args == false)) {samples = atoi(argv[i]), int_args = true; any = true;}
        else if((isdigit(*argv[i])) && (int_args == true)) {tdelay = atoi(argv[i]); any = true;}
        //if argument is a number, set tdelay to that number
        if((strncmp(argv[i], "--tdelay=", 9) == 0)){ tdelay = atoi(argv[i] + 9); any = true;} 
        if((strncmp(argv[i], "-td=", 4) == 0)){ tdelay = atoi(argv[i] + 4); any = true;}
        
    }
    //if no arguments are entered, print all stats
    if((any == true) || (argc ==1) ){ finPrint(sys, user, graph, sequen, samples, tdelay);}
    //if invalid arguments are entered, print error message
    else{ printf("Please enter valid arguments\n");}

    return 0;
}

