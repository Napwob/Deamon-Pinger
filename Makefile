all:
	gcc -Wall -o main main.c ping_stat.c icmp_ping.c deamon.c unix_socket.c -lpthread 
