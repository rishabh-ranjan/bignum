EXEC     = test
SRC_DIR  = src
DEBUG    = debug
RELEASE  = release

CC       = gcc
CFLAGS   = -std=gnu11 -O3 -Wall -Wextra -Wpedantic -Wstrict-aliasing
DFLAGS   = -DDEBUG

SRC      = $(shell find $(SRC_DIR) -type f -name *.c | xargs basename)
OBJ      = $(SRC:.c=.o)

all: $(DEBUG)/$(EXEC) $(RELEASE)/$(EXEC)

# may not work for multiple c files
$(DEBUG)/$(EXEC): $(DEBUG)/$(OBJ)
	$(CC) -o $@ $^

$(RELEASE)/$(EXEC): $(RELEASE)/$(OBJ)
	$(CC) -o $@ $^

$(DEBUG)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS) $(DFLAGS)

$(RELEASE)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: drun
drun: $(DEBUG)/$(EXEC)
	$(DEBUG)/$(EXEC)

.PHONY: rrun
rrun: $(RELEASE)/$(EXEC)
	$(RELEASE)/$(EXEC)

