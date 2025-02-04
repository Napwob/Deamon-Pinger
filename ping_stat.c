#include "ping_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

static PingStat *head = NULL;

void PingStat_add(const struct in_addr *addr, int sent, int received) {
    if (!addr) return;

    PingStat *new_entry = (PingStat *)malloc(sizeof(PingStat));
    if (!new_entry) {
        perror("Memory allocation error");
        exit(1);
    }
    new_entry->data.addr = *addr;
    new_entry->data.sent = sent;
    new_entry->data.received = received;
    new_entry->next = head;
    head = new_entry;
}

void PingStat_add_s(const PingData *stat) {
    if (!stat) return;
    PingStat_add(&stat->addr, stat->sent, stat->received);
}

void PingStat_update(const struct in_addr *addr, int sent, int received) {
    if (!addr) return;

    PingStat *current = head;
    while (current) {
        if (memcmp(&current->data.addr, addr, sizeof(struct in_addr)) == 0) {
            current->data.sent += sent;
            current->data.received += received;
            return;
        }
        current = current->next;
    }
    PingStat_add(addr, sent, received);
}

void PingStat_update_s(const PingData *stat) {
    if (!stat) return;
    PingStat_update(&stat->addr, stat->sent, stat->received);
}

void PingStat_print() {
    PingStat *current = head;
    while (current) {
        printf("%s: sent %d, recieve %d\n",
               inet_ntoa(current->data.addr), current->data.sent, current->data.received);
        current = current->next;
    }
}

void PingStat_free() {
    PingStat *current = head;
    while (current) {
        PingStat *temp = current;
        current = current->next;
        free(temp);
    }
}
