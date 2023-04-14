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

void INThandler(int sig)
{
    char c;
    signal(sig, SIG_IGN);
    printf("Do you really want to quit? [y/n]");
    c = getchar();
    if (c == 'y' || c == 'Y')
        exit(0);
    else
        signal(SIGINT, INThandler);
    getchar(); // Get new line character
}
void interceptZ(int signal) {
    // intercept ctrl-z
}

//Function to print all stats
void finPrint(bool sys, bool user, bool graph, bool sequen, int samples, int tdelay){
    
	
    // Create Pipes
    int pipefd_memory[2], pipefd_cpu[2], pipefd_user[2];

    //declare arrays to store memory and cpu graphical usage
    char memory_arr[samples][1024];
    char graph_arr[samples][1024];

    double prev_virt;
    long int prev_cpu = 0;
    long int prev_idle = 0;

    for (int i = 0; i < samples; i++)
    {
	    
        if (pipe(pipefd_memory) == -1 || pipe(pipefd_cpu) == -1 || pipe(pipefd_user) == -1) {
                fprintf(stderr, "Unable to create pipe (%s)\n", strerror(errno));
                exit(1);
            }

            pid_t pid_memory = fork();

        if (pid_memory == -1){
            fprintf(stderr, "Unable to fork (%s)\n", strerror(errno));
            exit(1);
        }
	    
        else if (pid_memory == 0) {
            // Child process
            close(pipefd_memory[0]); // Close unused read 
            close(pipefd_cpu[0]); close(pipefd_cpu[1]); close(pipefd_user[0]); close(pipefd_user[1]); 
            dup2(pipefd_memory[1], STDOUT_FILENO); // Redirect stdout to the pipe
                
            // Call Memory Function
            getMemory(pipefd_memory);
            exit(0);
            } 
        else {

            pid_t pid_users = fork();
            if(pid_users == -1){
                fprintf(stderr, "Unable to fork (%s)\n", strerror(errno));
                exit(1);
            }

            if (pid_users == 0) {
                // Child process
                close(pipefd_user[0]); // Close unused read 
                close(pipefd_memory[0]); close(pipefd_memory[1]); close(pipefd_cpu[0]); close(pipefd_cpu[1]); 
                
                // Call Users Function
                userPrint(pipefd_user);
                exit(0);
                } 
                else {

                    pid_t pid_cpu = fork();
                    if (pid_cpu == -1)
                    {
                        fprintf(stderr, "Unable to fork (%s)\n", strerror(errno));
                        exit(1);
                    }
                   
                   else if (pid_cpu == 0) {
                            // Child process
                        close(pipefd_cpu[0]); // Close unused read 
                        close(pipefd_memory[0]); close(pipefd_memory[1]); close(pipefd_user[0]); close(pipefd_user[1]); 
                        dup2(pipefd_cpu[1], STDOUT_FILENO); // Redirect stdout to the pipe
                    
                        // Call Cpu Function
                        getCpu(pipefd_cpu);
                        exit(0);
                        } 
                    else {
                        // Main partent Process
                        // Close the write end of the pipe for the parent
                        close(pipefd_cpu[1]); close(pipefd_memory[1]); close(pipefd_user[1]);
                        // Wait for information retrival
                        waitpid(pid_memory, NULL, 0);
                        waitpid(pid_users, NULL, 0);
                        waitpid(pid_cpu, NULL, 0);

                        //if sequen is true, print iteration number
                        if(sequen){
                            printf(">>> iteration %d\n", i);
                            starter(samples, tdelay);
                            if(sys){
                                struct mem_stat* mem;
                                struct cpu_stat* cpu;
                                read(pipefd_memory[0], mem, sizeof(struct mem_stat));
                                read(pipefd_cpu[0], cpu, sizeof(struct cpu_stat));
                                memoryPrint(memory_arr, samples, graph, i, &prev_virt, mem);
                                cpuPrint(graph_arr, tdelay, samples, graph, i, &prev_cpu, &prev_idle, cpu);
                                close(pipefd_cpu[0]); close(pipefd_memory[0]); close(pipefd_user[0]);
                            }
                            else if(user && !sys){
                                //userPrint();
                                sleep(tdelay);
                            }
                            else if(user && sys){
                                struct mem_stat* mem;
                                struct cpu_stat* cpu;
                                read(pipefd_memory[0], mem, sizeof(struct mem_stat));
                                read(pipefd_cpu[0], cpu, sizeof(struct cpu_stat));
                                memoryPrint(memory_arr, samples, graph, i, &prev_virt, mem);
                                //userPrint();
                                cpuPrint(graph_arr, tdelay, samples, graph, i, &prev_cpu, &prev_idle, cpu);
                                close(pipefd_cpu[0]); close(pipefd_memory[0]); close(pipefd_user[0]);
                            }
                        }
                        else{
                            printf("\033[H \033[2J \n");
                            starter(samples, tdelay);
                            if(sys && !user){
                                struct mem_stat* mem;
                                struct cpu_stat* cpu;
                                read(pipefd_memory[0], mem, sizeof(struct mem_stat));
                                read(pipefd_cpu[0], cpu, sizeof(struct cpu_stat));
                                memoryPrint(memory_arr, samples, graph, i, &prev_virt, mem);
                                cpuPrint(graph_arr, tdelay, samples, graph, i, &prev_cpu, &prev_idle, cpu);
                                close(pipefd_cpu[0]); close(pipefd_memory[0]); close(pipefd_user[0]);
                            }
                            else if(user && !sys){
                                //userPrint();
                                sleep(tdelay);
                            }
                            else if(user && sys){
                                
                                struct mem_stat* mem;
                                struct cpu_stat* cpu;
                                read(pipefd_memory[0], mem, sizeof(struct mem_stat));
                                read(pipefd_cpu[0], cpu, sizeof(struct cpu_stat));
                                memoryPrint(memory_arr, samples, graph, i, &prev_virt, mem);
                                //printf(">>> iteration %d\n", i);
                                close(pipefd_memory[0]);
                                //userPrint();
                                cpuPrint(graph_arr, tdelay, samples, graph, i, &prev_cpu, &prev_idle, cpu);
                                close(pipefd_cpu[0]);  close(pipefd_user[0]);      
                            }
                        }
                        sleep(tdelay);
                        ender();
                }
            }
        }
    }    
}


//Main function where arguments are parsed
int main(int argc, char *argv[]){
    signal(SIGTSTP, interceptZ);
    signal(SIGINT, INThandler);
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
