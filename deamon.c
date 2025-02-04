#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h> 
#include "ping_stat.h"

#define PID_FILE "/var/run/ping_daemon.pid"
int pid_fd;

static void remove_pid_file()
{
    if (pid_fd != -1)
    {
        close(pid_fd);
        unlink(PID_FILE);
    }
}

static void free_and_exit()
{
    remove_pid_file();
    PingStat_free();
    exit(1);
}

int check_and_create_pid_file()
{
    pid_fd = open(PID_FILE, O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (pid_fd < 0)
    {
        perror("Error creating PID file");
        return -1;
    }

    char pid_str[10];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    if (write(pid_fd, pid_str, strlen(pid_str)) < 0)
    {
        perror("Error writing PID to file");
        return -1;
    }
    return 0;
}

static void handle_exit(int sig)
{
    free_and_exit();
}

void daemonize()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    if (pid > 0)
    {
        exit(0);
    }

    if (setsid() < 0)
    {
        perror("setsid failed");
        exit(1);
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    if (pid > 0)
    {
        exit(0);
    }

    chdir("/");
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGTERM, handle_exit);
    signal(SIGINT, handle_exit);
}
