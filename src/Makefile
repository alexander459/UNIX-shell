CC=gcc
CFLAGS=-g

all: shell.out

shell.out: main.o shell.o
	$(CC) $(CFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f *.out *.o
