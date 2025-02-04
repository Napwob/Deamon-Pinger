#ifndef DAEMON_H
#define DAEMON_H

void daemonize();
int check_and_create_pid_file();
void free_and_exit();
void handle_exit(int sig);

#endif
