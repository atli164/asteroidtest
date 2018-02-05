CC := g++
CFLAGS := -std=gnu++17 
LIBS := `pkg-config --libs allegro-5 allegro_image-5 allegro_primitives-5`
SOURCE := main.cpp
BIN := asteroids

all:
	$(CC) $(SOURCE) $(FLAGS) $(LIBS) -o $(BIN)
