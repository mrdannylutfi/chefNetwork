CC = gcc
CFLAGS = -Wall -Wextra -lhiredis

all: competitor_node scoreboard_display

competitor_node: competitor_node.c
	$(CC) competitor_node.c -o competitor_node $(CFLAGS)

scoreboard_display: scoreboard_display.c
	$(CC) scoreboard_display.c -o scoreboard_display $(CFLAGS)

clean:
	rm -f competitor_node scoreboard_display competition.db
