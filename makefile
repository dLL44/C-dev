CC = g++
CFLAGS = -std=c++11 -Wall -I./include
LDFLAGS = -L./lib -lmingw32 -lSDL2main -lSDL2 -mconsole

all: untiled

untiled: src/main.cpp
	$(CC) $(CFLAGS) -o untiled src/main.cpp $(LDFLAGS)

clean:
	rm -f untiled
