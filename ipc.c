#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ERR_PIPE_FAIL 1
#define ERR_FORK_FAIL 2
#define ERR_WRITE_FAIL 3
#define ERR_READ_FAIL 4

static void close_if_open(int fd);

int main(void)
{
    int p2c[2];
    int c2p[2];

    if (pipe(p2c) == -1)
    {
        perror("pipe !!");
        return ERR_PIPE_FAIL;
    }
    if (pipe(c2p) == -1)
    {
        perror("pipe!!");
        close_if_open(p2c[0]);
        close_if_open(p2c[1]);
        return ERR_PIPE_FAIL;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork !!");
        close_if_open(p2c[0]);
        close_if_open(p2c[1]);
        close_if_open(c2p[0]);
        close_if_open(c2p[1]);
        return ERR_FORK_FAIL;
    }

    if (pid == 0)
    {

        close_if_open(p2c[1]);
        close_if_open(c2p[0]);

        char buffer[256];
        ssize_t n = read(p2c[0], buffer, sizeof(buffer) - 1);
        if (n < 0)
        {
            perror("child read !!");
            exit(ERR_READ_FAIL);
        }
        buffer[n] = '\0';
        printf("%s", buffer);

        char msg[128];
        int len = snprintf(msg, sizeof(msg),
                           "Daddy, my name is %d\n", getpid());

        if (write(c2p[1], msg, len) != len)
        {
            perror("child write!!");
            exit(ERR_WRITE_FAIL);
        }

        close_if_open(p2c[0]);
        close_if_open(c2p[1]);

        exit(0);
    }

    close_if_open(p2c[0]);
    close_if_open(c2p[1]);

    char pmsg[128];
    int pmsg_len = snprintf(pmsg, sizeof(pmsg),
                            "I am your daddy! and my name is %d\n", getpid());

    if (write(p2c[1], pmsg, pmsg_len) != pmsg_len)
    {
        perror("parent write!!");
        close_if_open(p2c[1]);
        close_if_open(c2p[0]);
        return ERR_WRITE_FAIL;
    }

    char buffer[256];
    ssize_t r = read(c2p[0], buffer, sizeof(buffer) - 1);
    if (r < 0)
    {
        perror("parent read");
        close_if_open(p2c[1]);
        close_if_open(c2p[0]);
        return ERR_READ_FAIL;
    }
    buffer[r] = '\0';
    printf("%s", buffer);

    close_if_open(p2c[1]);
    close_if_open(c2p[0]);

    wait(NULL);

    return 0;
}

static void close_if_open(int fd)
{
    if (fd >= 0)
        close(fd);
}
