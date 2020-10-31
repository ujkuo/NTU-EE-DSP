.PHONY: all clean

CC = g++
CFLAGS+=
LDFLAGS+=-lm

TARGET=test train

all: $(TARGET)

train: src/train.cpp
		$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -Iinc

test: src/test.cpp
		$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -Iinc
	
clean:
	$(RM) $(TARGET)
