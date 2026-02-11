default: client.c
	rm controltool -f
	gcc client.c -o controltool `sdl2-config --cflags --libs` -lSDL2_ttf

clean:
	rm controltool -f
