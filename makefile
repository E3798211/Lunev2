# Compile settings
CC=gcc
CFLAGS=-Wall -pthread -std=c11 -O2 -g
LFLAGS=-pthread

# Sources
SRC:=src
OBJ:=obj
BIN:=bin

SOURCES=$(wildcard $(SRC)/*.c)
HEADERS=$(wildcard $(SRC)/*.h)
OBJECTS=$(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SOURCES))

BINARY= $(BIN)/main

# Specific variables
debug:		CFLAGS +=-DDEBUG

# Check if directories created before any build
-include directories

# Targets
all: directories debug

debug release: $(HEADERS)

debug release: $(OBJECTS)
	$(CC) -o $(BINARY) $^ $(LFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

# Additional targets
.PHONY: clean directories

clean:
	rm -rf $(OBJ)

directories:
	if [ ! -d "$(OBJ)" ]; then mkdir "$(OBJ)"; fi
	if [ ! -d "$(BIN)" ]; then mkdir "$(BIN)"; fi

