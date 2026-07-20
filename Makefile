
CC = gcc
CFLAGS = -g -Wall -Wextra
TARGET = shell
SRC = Basic_UNIX_Shell.c

all: $(TARGET)
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean: 
	rm -f $(TARGET)
