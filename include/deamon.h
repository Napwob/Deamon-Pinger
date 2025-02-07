#ifndef DAEMON_H
#define DAEMON_H

void Deamon_start();
int Deamon_check_pid_file();
int Deamon_create_pid_file();
void free_and_exit();

#endif
