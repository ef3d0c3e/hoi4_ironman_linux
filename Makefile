NAME := ironman.so
CC ?= gcc
CFLAGS := -Wall -Wextra -Werror -Wconversion -fpic -shared

SOURCES := ironman.c

# Objects
OBJECTS := $(SOURCES:.c=.o)

# Default target
$(NAME):
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

.PHONY: all
all: $(NAME)

.PHONY: clean
clean:
	$(RM) $(NAME)

.PHONY: re
re: clean all
