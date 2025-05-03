CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

SRCS = main.c match_allocator.c utils.c bloom_filter.c bloom_filter_utils.c
OBJS = $(SRCS:.c=.o)
TARGET = freelancer_matcher

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 