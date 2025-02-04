all:
	gcc -Wall -o main main.c ping_stat.c icmp_ping.c -lpthread 
