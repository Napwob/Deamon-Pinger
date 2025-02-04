#include "ping_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

static PingStat *head = NULL;
static pthread_mutex_t stat_mutex = PTHREAD_MUTEX_INITIALIZER;

void PingStat_add(const struct in_addr *addr, int sent, int received)
{
    if (!addr)
        return;

    PingStat *new_entry = (PingStat *)malloc(sizeof(PingStat));
    if (!new_entry)
    {
        perror("Memory allocation error");
        exit(1);
    }

    new_entry->data.addr = *addr;
    new_entry->data.sent = sent;
    new_entry->data.received = received;

    pthread_mutex_lock(&stat_mutex);
    new_entry->next = head;
    head = new_entry;
    pthread_mutex_unlock(&stat_mutex);
}

void PingStat_add_s(const PingData *stat)
{
    if (!stat)
        return;
    PingStat_add(&stat->addr, stat->sent, stat->received);
}

void PingStat_update(const struct in_addr *addr, int sent, int received)
{
    if (!addr)
        return;

    pthread_mutex_lock(&stat_mutex);

    PingStat *current = head;
    while (current)
    {
        if (memcmp(&current->data.addr, addr, sizeof(struct in_addr)) == 0)
        {
            current->data.sent += sent;
            current->data.received += received;
            pthread_mutex_unlock(&stat_mutex);
            return;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&stat_mutex);
    PingStat_add(addr, sent, received);
}

void PingStat_update_s(const PingData *stat)
{
    if (!stat)
        return;
    PingStat_update(&stat->addr, stat->sent, stat->received);
}

void PingStat_print()
{
    pthread_mutex_lock(&stat_mutex);

    PingStat *current = head;
    while (current)
    {
        printf("%s: sent %d, received %d\n",
               inet_ntoa(current->data.addr), current->data.sent, current->data.received);
        current = current->next;
    }

    pthread_mutex_unlock(&stat_mutex);
}

void PingStat_socket_write(int fd)
{
    pthread_mutex_lock(&stat_mutex);

    PingStat *current = head;
    char buffer[4096];
    int offset = 0;

    while (current)
    {
        int len = snprintf(buffer + offset, sizeof(buffer) - offset,
                           "%s: sent %d, received %d\n",
                           inet_ntoa(current->data.addr),
                           current->data.sent,
                           current->data.received);

        if (len < 0 || offset + len >= sizeof(buffer))
        {
            break;
        }

        offset += len;
        current = current->next;
    }

    pthread_mutex_unlock(&stat_mutex);

    if (offset > 0)
    {
        write(fd, buffer, offset);
    }
}

void PingStat_free()
{
    pthread_mutex_lock(&stat_mutex);

    PingStat *current = head;
    while (current)
    {
        PingStat *temp = current;
        current = current->next;
        free(temp);
    }

    head = NULL;
    pthread_mutex_unlock(&stat_mutex);
}
