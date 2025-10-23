CC := gcc
CFLAGS := -O2 -Wall -Wextra -std=c11
SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
BIN := physics_sim

# Use pkg-config for GLFW and GLEW if available
CFLAGS += $(shell pkg-config --cflags glfw3 glew 2>/dev/null)
LDFLAGS := $(shell pkg-config --libs glfw3 glew 2>/dev/null) -lGL -lm -ldl -lpthread

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(BIN)

run: $(BIN)
	./$(BIN)

.PHONY: all clean run
