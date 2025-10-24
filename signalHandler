#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>


volatile sig_atomic_t debug_mode = 0;
volatile sig_atomic_t terminate_program = 0;


void handle_sigint(int sig) {
    (void)sig; 
    debug_mode = !debug_mode;  
}


void handle_sigusr1(int sig) {
    (void)sig; 
    terminate_program = 1;
}

int main(void) {
    struct sigaction sa_int, sa_usr1;
    sa_int.sa_handler = handle_sigint;
    sa_usr1.sa_handler = handle_sigusr1;

    sigemptyset(&sa_int.sa_mask);
    sigemptyset(&sa_usr1.sa_mask);

    sa_int.sa_flags = 0;
    sa_usr1.sa_flags = 0;

    
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("Error in registering SIGINT handler");
        return 1;
    }

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("Error in registering SIGUSR1 handler");
        return 2;
    }

    unsigned long iteration = 0;

    while (!terminate_program) {
        iteration++;
        if (debug_mode) {
            printf("Iteration :  %lu\n", iteration);
            fflush(stdout); 
        }
        sleep(2);
    }

    
    printf("\nProgram has terminated gracefully after %lu iterations.\n", iteration);
    return 0;
}
