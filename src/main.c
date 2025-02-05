#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "ping_stat.h"
#include "icmp_ping.h"
#include "deamon.h"
#include "unix_socket.h"

#define SOCKET_PATH "/tmp/usock"

static void handle_arguments(int argc, char *argv[]);
static void setup_daemon_mode();
static void process_client_request(int client, char *buffer);
static void handle_ping_command(int client, char *buffer);
static void start_server(int unix_socket);

int main(int argc, char *argv[]) {
    handle_arguments(argc, argv);
    
    remove_socket_if_exists();
    
    int unix_socket = create_unix_socket(SOCKET_PATH);
    if (unix_socket < 0) {
        perror("Error creating UNIX socket");
        exit(EXIT_FAILURE);
    }
    
    start_server(unix_socket);
    
    free_and_exit();
    return EXIT_SUCCESS;
}

static void handle_arguments(int argc, char *argv[]) {
    if (check_and_create_pid_file() < 0) {
        exit(EXIT_FAILURE);
    }

    if (argc == 2) {
        if (strncmp(argv[1], "--debug", 7) == 0) {
            puts("Debug mode");
        } else if (strncmp(argv[1], "--help", 6) == 0) {
            puts("ping command: \"echo \"ping <ip_addr> <ping_count>\" | nc -U /tmp/usock\"");
            puts("show command: \"echo \"show\" | nc -U /tmp/usock\"");
            puts("debugmode arg: --debug");
            exit(EXIT_SUCCESS);
        }
    } else {
        setup_daemon_mode();
    }
}

static void setup_daemon_mode() {
    puts("Deamon mode");
    daemonize();
}

static void start_server(int unix_socket) {
    while (1) {
        int client = accept(unix_socket, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) continue;
            perror("accept error");
            break;
        }
        
        char buffer[256] = {0};
        ssize_t rec_len = read(client, buffer, sizeof(buffer) - 1);
        if (rec_len < 0) {
            perror("read error");
            close(client);
            continue;
        }
        buffer[rec_len] = '\0';
        
        process_client_request(client, buffer);
        close(client);
    }
}

static void process_client_request(int client, char *buffer) {
    if (strncmp(buffer, "ping", 4) == 0) {
        handle_ping_command(client, buffer);
    } else if (strncmp(buffer, "show", 4) == 0) {
        PingStat_socket_write(client);
    } else {
        if (write(client, "Unknown command\n", 17) < 0) {
            perror("write error");
        }
    }
}

static void handle_ping_command(int client, char *buffer) {
    int ping_number;
    char ip_addr[16];
    
    if (parse_ping_command(buffer, ip_addr, &ping_number) == 0) {
        PingThreadArgs *args = malloc(sizeof(PingThreadArgs));
        if (!args) {
            perror("malloc error");
            return;
        }
        snprintf(args->ip_addr, sizeof(args->ip_addr), "%s", ip_addr);
        args->ping_count = ping_number;
        
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, ping_worker, args) != 0) {
            perror("pthread_create error");
            free(args);
        } else {
            pthread_detach(thread_id);
            if (write(client, "ok\n", 3) < 0) {
                perror("write error");
            }
        }
    } else {
        if (write(client, "Invalid ping command\n", 21) < 0) {
            perror("write error");
        }
    }
}