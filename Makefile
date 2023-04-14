CC = gcc
CFLAGS = -Wall -Wextra -Werror
BINARY=A3main

.PHONY: all
all: $(BINARY)
## Compile executable
A3main: A3main.o stats_functions.o
	$(CC) $(CFLAGS) -o $@ $^

## Compile files to create object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< 

## clean: used to remove object files
.PHONY: clean
clean:
	rm -f *.o A3main.c

## Help: help function called
.PHONY: help
help: Makefile
	@sed -n 's/^##//p' $<
