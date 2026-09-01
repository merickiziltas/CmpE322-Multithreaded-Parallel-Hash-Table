# Compiler & Flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread

# Targets and Sources
TARGET = test_main
SRCS = test_main.c hash_parallelization.c
OBJS = $(SRCS:.c=.o)

# Default Rule
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
