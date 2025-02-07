#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

extern int unix_socket;

typedef struct
{
    char ip_addr[16];
    int ping_count;
} PingThreadArgs;

int create_unix_socket(const char *sock_path);
void *ping_worker(void *args);
void remove_socket_if_exists();

#endif
