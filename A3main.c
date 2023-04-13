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





//Function to print all stats
void finPrint(bool sys, bool user, bool graph, bool sequen, int samples, int tdelay){
    signal(SIGTSTP, CtrlZ);
    signal(SIGINT, CtrlC);
    MemStruct memStruct;
    CpuStruct cpuStruct;
    //declare arrays to store memory and cpu graphical usage
    char memory_arr [samples][200];
    char graph_arr[samples][200];
    for (int i = 0; i < samples; i++) {
        strcpy(memory_arr[i], "");
        strcpy(graph_arr[i], "");
    }
    char mem_arr[MAX_LEN];
    char cpu_arr[MAX_LEN];
    char user_arr[MAX_LEN];
   
    int temp = 0;
    for (int i = 0; i < samples; i++)
    {
        pid_t pid[3];
        int cpuPipe[2];
        int memPipe[2];
        int userPipe[2];
        if(pipe(cpuPipe) != 0 || pipe(memPipe) != 0 || pipe(userPipe) != 0){
            fprintf(stderr, "Pipe failed\n");
            exit(1);
        }
        strcpy(mem_arr, "");
        strcpy(user_arr, "");
        strcpy(cpu_arr, "");
        for(int j = 0; j < 3; j++){
             if (j == 0) {
                if ((pid[j] = fork()) == -1){
                    fprintf(stderr, "Fork failed\n");
                }
                else if (pid[j] == 0) {
                    close(cpuPipe[0]);
                    getCpu(cpu_arr, &cpuStruct.prevTime, &cpuStruct.currCpu, &cpuStruct.prevUtil, graph, i);
                    temp = write(cpuPipe[1], &cpuStruct, sizeof(cpuStruct));
                    if (temp == -1){
                        fprintf(stderr, "Write to pipe failed\n");
                        exit(0);
                    }
                    close(cpuPipe[0]);
                }
                else {
                    while(wait(NULL) > 0);
                    close(cpuPipe[1]);
                    temp = read(cpuPipe[0], &cpuStruct, sizeof(cpuStruct));
                    if (temp == -1){
                        fprintf(stderr, "Read from pipe failed\n");
                        exit(0);
                    }
                    strcpy(graph_arr[i], cpuStruct.cpuString);
                    close(cpuPipe[0]);
                }
            }
            else if (j == 1) {
                if ((pid[j] = fork()) == -1){
                    fprintf(stderr, "Fork failed\n");
                }
                else if (pid[j] == 0) {
                    close(memPipe[0]);
                    getMem(memory_arr, graph, i, &memStruct.prevMem);
                    temp = write(memPipe[1], &memStruct, sizeof(memStruct));
                    if (temp == -1){
                        fprintf(stderr, "Write to pipe failed\n");
                        exit(0);
                    }
                    close(memPipe[1]);
                }
                else {
                    while(wait(NULL) > 0);
                    close(memPipe[1]);
                    temp = read(memPipe[0], &memStruct, sizeof(memStruct));
                    if (temp == -1){
                        fprintf(stderr, "Read from pipe failed\n");
                        exit(0);
                    }
                    strcpy(memory_arr[i], memStruct.memString);
                    close(memPipe[0]);
                }
            }
            else if (j == 2) {
                if ((pid[j] = fork())== -1){
                    fprintf(stderr, "Fork failed\n");
                }
                else if (pid[j]== 0) {
                    close(userPipe[0]);
                    userPrint(user_arr);
                    temp = write(userPipe[1], user_arr, strlen(user_arr)+1);
                    if (temp == -1){
                        fprintf(stderr, "Write to pipe failed\n");
                        exit(0);
                    }
                    close(userPipe[1]);
                    
                }
                else {
                    while(wait(NULL) > 0);
                    close(userPipe[1]);
                    temp = read(userPipe[0], &memStruct, sizeof(memStruct));
                    if (temp == -1){
                        fprintf(stderr, "Read from pipe failed\n");
                        exit(0);
                    }
                    close(userPipe[0]);
                }
            }
        }

        //if sequen is true, print iteration number
        if(sequen){
            printf(">>> iteration %d\n", i);
            starter(samples, tdelay);
            if(sys && !user){
                memPrint(memory_arr, samples, graph, i);
                cpuPrint(graph_arr, tdelay, samples, cpuStruct.currCpu, i);
            }
            else if(user && !sys){
                userPrint();
                sleep(tdelay);
            }
            else if(user && sys){
                memPrint(memory_arr, samples, graph, i);
                userPrint();
                cpuPrint(graph_arr, tdelay, samples, cpuStruct.currCpu, i);
            }
        }
        else{
            printf("\033[H \033[2J \n");
            starter(samples, tdelay);
            if(sys && !user){
                memPrint(memory_arr, samples, graph, i);
                cpuPrint(graph_arr, tdelay, samples, cpuStruct.currCpu, i);
            }
            else if(user && !sys){
                userPrint();
                sleep(tdelay);
            }
            else if(user && sys){
                memPrint(memory_arr, samples, graph, i);
                
                userPrint();
                cpuPrint(graph_arr, tdelay, samples, cpuStruct.currCpu, i);     
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

