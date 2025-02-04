#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

extern int unix_socket;

typedef struct
{
    char ip_addr[16];
    int ping_count;
} PingThreadArgs;

int create_unix_socket(char *sock_path);
int parse_ping_command(const char *buffer, char *ip_addr, int *ping_count);
void *ping_worker(void *args);
void remove_socket_if_exists();

#endif
