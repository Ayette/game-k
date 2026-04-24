CC=gcc
CFLAGS=`sdl2-config --cflags`
LIBS=`sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf
SRC=main.c option.c puzzle.c
OUT=app

all:
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LIBS)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
