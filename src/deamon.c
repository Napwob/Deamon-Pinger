#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include "ping_stat.h"

#define PID_FILE "/var/run/ping_daemon.pid"

int pid_fd = -1;

static void remove_pid_file()
{
    if (pid_fd >= 0)
    {
        close(pid_fd);
        unlink(PID_FILE);
    }
}

void free_and_exit()
{
    remove_pid_file();
    PingStat_free();
    exit(EXIT_SUCCESS);
}

static void handle_exit(int sig)
{
    free_and_exit();
}

int Deamon_create_pid_file()
{
    int fd = open(PID_FILE, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        perror("Error opening PID file");
        return -1;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) < 0)
    {
        perror("Another instance is already running");
        close(fd);
        return -1;
    }

    char pid_str[10];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    ftruncate(fd, 0);
    if (write(fd, pid_str, strlen(pid_str)) < 0)
    {
        perror("Error writing PID to file");
        close(fd);
        return -1;
    }
    fsync(fd);

    pid_fd = fd;

    atexit(remove_pid_file);
    return 0;
}

void Deamon_start()
{
    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
    {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, handle_exit);
    signal(SIGINT, handle_exit);

    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
