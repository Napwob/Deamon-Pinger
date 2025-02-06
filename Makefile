CC = gcc
CFLAGS = -Wall -I./include
LDFLAGS = -lpthread

SRC = src/main.c src/ping_stat.c src/icmp_ping.c src/deamon.c src/unix_socket.c src/server.c
OBJ = $(SRC:.c=.o)
EXEC = dping

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)