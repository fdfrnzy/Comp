# Compiler and flags
CC = cc
CFLAGS = -std=c99 -Wall -Wextra -O2

# Target binary
TARGET = comp

# Source files
SRCS = lexer.c parser.c symbols.c compiler.c CodeGrader.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default rule
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)