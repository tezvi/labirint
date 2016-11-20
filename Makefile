CC = gcc
CFLAGS = -Wall -O2 -ggdb3
LDFLAGS = -lncurses -lmenu -L./
SRCS = labirintw.c game.c
HDRS = game.h
OBJS = $(SRCS:.c=.o)
PROG = labirint

all: $(PROG)

$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

$(OBJS) : $(HDRS)

clean:
	rm -f *.o $(PROG)
