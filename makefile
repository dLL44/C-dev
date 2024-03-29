CC = g++
CFLAGS = -std=c++11 -Wall -I./include
LDFLAGS = -L./lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -mconsole

all: untiled

untiled: src/main.cpp
	$(CC) $(CFLAGS) -o bin/app src/main.cpp $(LDFLAGS)

clean:
	rm -f untiled
