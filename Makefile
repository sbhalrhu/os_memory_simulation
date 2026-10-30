CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g

TARGET := memSim
SOURCE := memSim.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
