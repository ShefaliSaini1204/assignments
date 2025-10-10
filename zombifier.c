#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t sigcont_receive = 0;
static void sigcont_handler(int signumber){ (void)signumber; sigcont_receive = 1; }

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-n") != 0) {
        fprintf(stderr, "error as input paramters are incomplete. %s -n <positive_integer>\n", argv[0]);
        return 1;
    }

    errno = 0;
    char *endptr = NULL;
    long number_zombies = strtol(argv[2], &endptr, 10);
    if (errno || *endptr != '\0' || number_zombies <= 0) {
        fprintf(stderr, "Error: Input does not have invalid positive integer '%s'\n", argv[2]);
        return 1;
    }

    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigcont_handler;
    if (sigaction(SIGCONT, &sa, NULL) == -1) {
        perror("sigaction error");
        return 2;
    }

    
    sigset_t block, prev, waitset;
    sigemptyset(&block);
    sigaddset(&block, SIGCONT);
    if (sigprocmask(SIG_BLOCK, &block, &prev) == -1) {
        perror("sigprocmask error");
        return 2;
    }

    long spawn = 0;
    for (long i = 0; i < number_zombies; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("forked error");
            break;                      
        }
        if (pid == 0) {
            _exit(0);                   
        }
        ++spawn;
    }

    printf("Parent PID is %d. Created - %ld zombie(s).\n", getpid(), spawn);
    printf("Send SIGCONT to clean up the space: kill -CONT %d\n", getpid());
    fflush(stdout);

    
    waitset = prev;                     
    sigdelset(&waitset, SIGCONT);       
    while (!sigcont_receive) {
        sigsuspend(&waitset);
    }

    
    while (waitpid(-1, NULL, 0) > 0) { }

    if(spawn == number_zombies){return 0;}
    else {return 3;}
}
