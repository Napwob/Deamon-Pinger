all:
	gcc -Wall -o -lpthread main main.c ping_stat.c icmp_ping.c
