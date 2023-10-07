CC = gcc
CFLAGS = -Wall -g

SRC_DIR = src
HDR_DIR = headers

SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(HDR_DIR)/*.h)
OBJECTS = $(SOURCES:.c=.o)

MAIN_SOURCE = main.c
TARGET = main
DATA = account.txt

all: run

run: $(OBJECTS) $(MAIN_SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(MAIN_SOURCE)
	./$(TARGET) $(DATA)

compile: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(MAIN_SOURCE)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)