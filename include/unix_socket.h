#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

extern int unix_socket;

typedef struct
{
    char ip_addr[16];
    int ping_count;
} PingThreadArgs;

int UnixSocket_create(const char *sock_path);
void *UnixSocket_ping_worker(void *args);
void UnixSocket_remove_if_exists();

#endif
