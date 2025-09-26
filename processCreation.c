#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sigintHandler(int signop)
{
    (void)signop;
}

int main()
{
    struct sigaction ign = {0};
    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    ign.sa_flags= 0;
    sigaction(SIGINT, &ign, NULL);

    pid_t pid;
    pid = fork();

    if(pid < 0)
    {
        fprintf(stderr, "Fork failed.\n");
        return 1;
    }
    else if(pid == 0)
    {
        struct sigaction catch = {0};
        catch.sa_handler = sigintHandler;
        sigemptyset(&catch.sa_mask);
        catch.sa_flags= 0;
        sigaction(SIGINT, &catch, NULL);

        printf("%d\n", (int)getpid());
        fflush(stdout);

        pause();

        _exit(5);
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status))
        {
            printf("childpid=%d,exitstatus=%d\n", (int)pid, WEXITSTATUS(status));
        }
        else if(WIFSIGNALED(status))
        {
            printf("Signal killed the child process. Signal=%d\n", WTERMSIG(status));
        }
        else
        {
            printf("The Child proces did not exit normally.\n");
        }
    }

    return 0;
}
