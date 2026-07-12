CFLAGS ?= -O2 -Wall
SRCS := $(wildcard *.c)
BINS := $(SRCS:.c=)

all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< -lpthread

clean:
	rm -f $(BINS)

.PHONY: all clean
