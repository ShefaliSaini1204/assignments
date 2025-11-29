#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SHM_NAME      "/pc_shm"
#define SEM_MUTEX     "/pc_mutex"
#define SEM_EMPTY     "/pc_empty"
#define SEM_FULL      "/pc_full"
#define SOCKET_PATH   "/tmp/pc_socket"
#define MAX_QUEUE     1024
#define MAX_MSG_LEN   256

#define ERR_ARGS          1
#define ERR_SHM           2
#define ERR_SEM           3
#define ERR_SOCKET_MODE   4

struct shm_queue {
    int capacity;
    int head;
    int tail;
    int count;
    char buffer[MAX_QUEUE][MAX_MSG_LEN];
};

static int g_is_creator = 0;
static int g_shm_fd = -1;
static struct shm_queue *g_queue = NULL;
static sem_t *g_mutex = NULL;
static sem_t *g_empty = NULL;
static sem_t *g_full  = NULL;

static int g_socket_fd = -1;
static int g_socket_is_server = 0;

static void cleanup(void);
static void sigint_handler(int sig);

static void print_usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s (-p | -c) (-s | -u) -q <depth> [-m \"msg\"] [-e]\n"
        "  -p           producer mode\n"
        "  -c           consumer mode\n"
        "  -s           use shared memory\n"
        "  -u           use unix socket\n"
        "  -q <depth>   queue depth\n"
        "  -m \"msg\"     message to produce\n"
        "  -e           echo output\n",
        prog);
}

static void safe_strcpy(char *dst, const char *src, size_t dst_size)
{
    if (dst_size == 0) return;
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static int init_shared_resources(int queue_depth)
{
    g_shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (g_shm_fd >= 0) {
        g_is_creator = 1;
        if (ftruncate(g_shm_fd, sizeof(struct shm_queue)) == -1) return -1;
    } else if (errno == EEXIST) {
        g_is_creator = 0;
        g_shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (g_shm_fd < 0) return -1;
    } else return -1;

    g_queue = mmap(NULL, sizeof(struct shm_queue),
                   PROT_READ | PROT_WRITE, MAP_SHARED, g_shm_fd, 0);
    if (g_queue == MAP_FAILED) return -1;

    if (g_is_creator) {
        memset(g_queue, 0, sizeof(struct shm_queue));
        g_queue->capacity = queue_depth;
    } else {
        if (g_queue->capacity != queue_depth) return -1;
    }

    g_mutex = sem_open(SEM_MUTEX,
                       g_is_creator ? (O_CREAT | O_EXCL) : 0, 0666, 1);
    if (g_mutex == SEM_FAILED) {
        if (!(errno == EEXIST && g_is_creator)) return -1;
        g_mutex = sem_open(SEM_MUTEX, 0);
    }
    if (g_mutex == SEM_FAILED) return -1;

    g_empty = sem_open(SEM_EMPTY,
                       g_is_creator ? (O_CREAT | O_EXCL) : 0,
                       0666, queue_depth);
    if (g_empty == SEM_FAILED) {
        if (!(errno == EEXIST && g_is_creator)) return -1;
        g_empty = sem_open(SEM_EMPTY, 0);
    }
    if (g_empty == SEM_FAILED) return -1;

    g_full = sem_open(SEM_FULL,
                      g_is_creator ? (O_CREAT | O_EXCL) : 0,
                      0666, 0);
    if (g_full == SEM_FAILED) {
        if (!(errno == EEXIST && g_is_creator)) return -1;
        g_full = sem_open(SEM_FULL, 0);
    }
    if (g_full == SEM_FAILED) return -1;

    return 0;
}

static void cleanup(void)
{
    if (g_mutex && g_mutex != SEM_FAILED) {
        sem_close(g_mutex);
        if (g_is_creator) sem_unlink(SEM_MUTEX);
    }
    if (g_empty && g_empty != SEM_FAILED) {
        sem_close(g_empty);
        if (g_is_creator) sem_unlink(SEM_EMPTY);
    }
    if (g_full && g_full != SEM_FAILED) {
        sem_close(g_full);
        if (g_is_creator) sem_unlink(SEM_FULL);
    }

    if (g_queue && g_queue != MAP_FAILED) munmap(g_queue, sizeof(struct shm_queue));
    if (g_shm_fd >= 0) close(g_shm_fd);
    if (g_is_creator) shm_unlink(SHM_NAME);

    if (g_socket_fd != -1) close(g_socket_fd);
    if (g_socket_is_server) unlink(SOCKET_PATH);
}

static void sigint_handler(int sig)
{
    (void)sig;
    cleanup();
    _exit(0);
}

static void run_shared_producer(const char *msg, int echo_flag)
{
    while (1) {
        sem_wait(g_empty);
        sem_wait(g_mutex);

        int idx = g_queue->tail;
        safe_strcpy(g_queue->buffer[idx], msg, MAX_MSG_LEN);
        g_queue->tail = (g_queue->tail + 1) % g_queue->capacity;
        if (g_queue->count < g_queue->capacity) g_queue->count++;

        if (echo_flag) {
            printf("%s\n", msg);
            fflush(stdout);
        }

        sem_post(g_mutex);
        sem_post(g_full);
    }
}

static void run_shared_consumer(int echo_flag)
{
    char msg[MAX_MSG_LEN];

    while (1) {
        sem_wait(g_full);
        sem_wait(g_mutex);

        int idx = g_queue->head;
        safe_strcpy(msg, g_queue->buffer[idx], sizeof(msg));
        g_queue->head = (g_queue->head + 1) % g_queue->capacity;
        if (g_queue->count > 0) g_queue->count--;

        if (echo_flag) {
            printf("%s\n", msg);
            fflush(stdout);
        }

        sem_post(g_mutex);
        sem_post(g_empty);
    }
}

static void run_socket_producer(const char *msg, int echo_flag, int q)
{
    (void)q;

    g_socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (g_socket_fd < 0) exit(ERR_SOCKET_MODE);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    safe_strcpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

    while (1) {
        char buf[MAX_MSG_LEN];
        safe_strcpy(buf, msg, sizeof(buf));

        if (sendto(g_socket_fd, buf, strlen(buf) + 1, 0,
                   (struct sockaddr *)&addr, sizeof(addr)) == -1)
            exit(ERR_SOCKET_MODE);

        if (echo_flag) {
            printf("%s\n", buf);
            fflush(stdout);
        }

        usleep(100000);
    }
}

static void run_socket_consumer(int echo_flag, int q)
{
    (void)q;

    g_socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (g_socket_fd < 0) exit(ERR_SOCKET_MODE);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    safe_strcpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

    unlink(SOCKET_PATH);

    if (bind(g_socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        exit(ERR_SOCKET_MODE);

    g_socket_is_server = 1;

    char buf[MAX_MSG_LEN];

    while (1) {
        ssize_t n = recvfrom(g_socket_fd, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (n < 0) {
            if (errno == EINTR) continue;
            exit(ERR_SOCKET_MODE);
        }

        buf[n] = '\0';

        if (echo_flag) {
            printf("%s\n", buf);
            fflush(stdout);
        }
    }
}

int main(int argc, char *argv[])
{
    int is_producer = 0, is_consumer = 0;
    int use_shared = 0, use_socket = 0;
    int queue_depth = 0, echo_flag = 0;
    const char *message = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "pcsum:q:e")) != -1) {
        switch (opt) {
        case 'p': is_producer = 1; break;
        case 'c': is_consumer = 1; break;
        case 's': use_shared = 1; break;
        case 'u': use_socket = 1; break;
        case 'm': message = optarg; break;
        case 'q': queue_depth = atoi(optarg); break;
        case 'e': echo_flag = 1; break;
        default: print_usage(argv[0]); return ERR_ARGS;
        }
    }

    if ((is_producer + is_consumer) != 1) {
        fprintf(stderr, "Error: must specify exactly one of -p or -c.\n");
        print_usage(argv[0]);
        return ERR_ARGS;
    }

    if ((use_shared + use_socket) != 1) {
        fprintf(stderr, "Error: must specify exactly one of -s or -u.\n");
        print_usage(argv[0]);
        return ERR_ARGS;
    }

    if (queue_depth <= 0 || queue_depth > MAX_QUEUE) {
        fprintf(stderr, "Error: invalid queue depth.\n");
        print_usage(argv[0]);
        return ERR_ARGS;
    }

    if (is_producer && (!message || !message[0])) {
        fprintf(stderr, "Error: -m \"msg\" required in producer mode.\n");
        print_usage(argv[0]);
        return ERR_ARGS;
    }

    atexit(cleanup);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    if (use_shared) {
        if (init_shared_resources(queue_depth) != 0) return ERR_SHM;
        if (is_producer) run_shared_producer(message, echo_flag);
        else run_shared_consumer(echo_flag);
    } else {
        if (is_producer) run_socket_producer(message, echo_flag, queue_depth);
        else run_socket_consumer(echo_flag, queue_depth);
    }

    return 0;
}
